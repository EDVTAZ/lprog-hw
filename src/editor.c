#include <msg.h>
#include <buffer.h>

#include <parson.h>
#include <ncurses.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <fcntl.h>

#define SERVER_NAME "localhost"
#define LOCALHOST "127.0.0.1"
#define CERT "b.crt"
#define PPORT 8888
#define OWN_CURS_ID 0 
#define KEY_ESC 27

int user_id;
int file_id = 0;
buffer* b;

int hport = 64957;
struct pollfd poll_list[2];

int setup_ssl_connect(int local_port, char *server_name, int server_port)
{
	printf("server port: %d\n", server_port);
	printf("server name: %s\n", server_name);
	printf("local port: %d\n", local_port);
	// connects to server through socat ssl tunnel
	if( fork() == 0 )
	{
		char cmd[1024];
		snprintf(cmd, 1024, "socat tcp4-listen:%d,reuseaddr,fork ssl:%s:%d,cafile=%s,verify=1", local_port, server_name, server_port, CERT);
		system(cmd);
		return 0;
	}
	sleep(1);
	
    size_t addrlen;
    struct sockaddr_in address;
    int sock;

    //create socket
    if( ( sock = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
        perror("socket");
        return -1;
    }
    
    //set family, address, port
    memset( &address, 0, sizeof( address ) );
    address.sin_addr.s_addr = inet_addr( LOCALHOST );
    address.sin_family = AF_INET;
    address.sin_port = htons( local_port );
    //count address length
    addrlen = sizeof( address );
    
    //connect to server
    if( connect( sock, (struct sockaddr *)&address, addrlen ) < 0 )
    {
        perror( "connect" );
        return -1;
    }
	return sock;
}

char *create_login_payload(char *name, char *pass)
{
    JSON_Value *root_value = json_value_init_object( );
    JSON_Object *root_object = json_value_get_object( root_value );
    char *serialized_string = NULL;

    json_object_set_string( root_object, "name", name );
    json_object_set_string( root_object, "pass", pass );
    
    serialized_string = json_serialize_to_string( root_value );
    json_value_free( root_value );
    return serialized_string;
}

int handle_msg(int sock, message* msg)
{
    CMOVE_DIR dir;

    if(msg == NULL)
    {
        return 0;
    }

    switch(msg->type)
    {
        case MSG_OK:
            user_id = msg->user_id;
            send_msg(sock, create_msg(FILE_REQUEST, user_id, file_id, -1, NULL));
            break;
        case FILE_RESPONSE:
            b = buffer_deserialize(msg->payload, 1);
            ui_update(b->u);
            break;
        case INSERT:
            bcursor_insert(b, msg->user_id, msg->payload[0]);
            break;
        case INSERT_LINE:
            bcursor_insert_line(b, msg->user_id);
            break;
        case DELETE:
            bcursor_del(b, msg->user_id);
            break;
        case MOVE_CURSOR:
			if(msg->payload[0] == '0')
				dir = UP;
			if(msg->payload[0] == '1')
				dir = DOWN;
			if(msg->payload[0] == '2')
				dir = LEFT;
			if(msg->payload[0] == '3')
				dir = RIGHT;
            bcursor_move(b, msg->user_id, dir);
            break;
        case ADD_CURSOR:
            if(user_id != msg->user_id) bcursor_new(b, msg->user_id, b->first->id, 0);
            break;
        case DELETE_CURSOR:
            bcursor_free(b, msg->user_id);
            break;
        default:
            break;
            
    }
    //free msg
    delete_msg(msg);
}

int handle_input(int sock)
{
	int quit = 0;
    char payload[2];
    
    //read keyboard input
    int c = getch();
    
    switch(c){
        case KEY_LEFT:
            send_msg(sock, create_msg(MOVE_CURSOR, user_id, b->id, b->ver++, "2"));
            bcursor_move(b, OWN_CURS_ID, LEFT);
            break;
            
        case KEY_RIGHT:
            send_msg(sock, create_msg(MOVE_CURSOR, user_id, b->id, b->ver++, "3"));
            bcursor_move(b, OWN_CURS_ID, RIGHT);
            break;

        case KEY_UP:
            send_msg(sock, create_msg(MOVE_CURSOR, user_id, b->id, b->ver++, "0"));
            bcursor_move(b, OWN_CURS_ID, UP);
            break;

        case KEY_DOWN:
            send_msg(sock, create_msg(MOVE_CURSOR, user_id, b->id, b->ver++, "1"));
            bcursor_move(b, OWN_CURS_ID, DOWN);
            break;

        case KEY_BACKSPACE:
            send_msg(sock, create_msg(DELETE, user_id, b->id, b->ver++, NULL));
            bcursor_del(b, OWN_CURS_ID);
            break;
            
        case KEY_ESC:
            send_msg(sock, create_msg(QUIT, user_id, b->id, b->ver++, NULL));
            buffer_free(b);
            quit = 1;
            break;

        case '\n':
            send_msg(sock, create_msg(INSERT_LINE, user_id, b->id, b->ver++, NULL));
            bcursor_insert_line(b, OWN_CURS_ID);
            break;

        default:
            payload[0] = c;
            payload[1] = 0;
            send_msg(sock, create_msg(INSERT, user_id, b->id, b->ver++, payload));
            bcursor_insert(b, OWN_CURS_ID, c);
    }

	return quit;
}

int worker_loop(int sock)
{
    while(1)
    {
        //set poll list
        poll_list[0].fd = sock;
        poll_list[0].events = POLLIN;
        poll_list[1].fd = STDIN_FILENO;
        poll_list[1].events = POLLIN;
        
        if( poll( poll_list, 2, -1 ) > 0 )
        {
            //handle server socket message
            if( poll_list[0].revents & POLLIN )
            {
                //handle message
                handle_msg(sock, recv_msg(sock));
            }
            //handle stdin input
            if( poll_list[1].revents & POLLIN )
            {
                //handle keyboard input
                if( handle_input(sock) == -1 ) break;
            }
            
        }
    }

	return 0;
}

int main()
{
	int worker_port;

	// log in to main server
	int client_sock = setup_ssl_connect(hport++, SERVER_NAME, PPORT);
	char *payload = create_login_payload("pisti", "degec");
    message* msg = create_msg(LOGIN, -1, -1, -1, payload);
    send_msg( client_sock, msg );

	// receive worker port
	msg = recv_msg( client_sock );
	if( msg->type == MSG_OK )
	{
		// TODO worker port is only told after file is chosen -> move all this stuff into the loop and deal with reconnecting somehow
		sscanf(msg->payload, "M%d", &worker_port);
		user_id = msg->user_id;
	}
	else
	{
		print_msg(msg);
		printf("Server replied with not ok\n");
		return 1;
	}
	
	// log out from main server
	send_msg(client_sock, create_msg(QUIT, user_id, -1, -1, NULL));
	close(client_sock);

	// log in to worker
	client_sock = setup_ssl_connect(hport++, SERVER_NAME, worker_port);
	payload = create_login_payload("pisti", "degec");
	printf(payload);
    msg = create_msg(LOGIN, -1, -1, -1, payload);
    send_msg( client_sock, msg );

	worker_loop( client_sock );

    close( client_sock );
	return 0;
}
