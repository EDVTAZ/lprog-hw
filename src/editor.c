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

#define SERVER_NAME "152.66.157.54"
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
    
    //read keyboard input
    int c = getch();
	char* cc;
    
    switch(c){
        case KEY_LEFT:
			cc = malloc(2);
			cc[0]='2'; cc[1]=0;
            send_msg(sock, create_msg(MOVE_CURSOR, user_id, b->id, b->ver++, cc));
            bcursor_move(b, OWN_CURS_ID, LEFT);
            break;
            
        case KEY_RIGHT:
			cc = malloc(2);
			cc[0]='3'; cc[1]=0;
            send_msg(sock, create_msg(MOVE_CURSOR, user_id, b->id, b->ver++, cc));
            bcursor_move(b, OWN_CURS_ID, RIGHT);
            break;

        case KEY_UP:
			cc = malloc(2);
			cc[0]='0'; cc[1]=0;
            send_msg(sock, create_msg(MOVE_CURSOR, user_id, b->id, b->ver++, cc));
            bcursor_move(b, OWN_CURS_ID, UP);
            break;

        case KEY_DOWN:
			cc = malloc(2);
			cc[0]='1'; cc[1]=0;
            send_msg(sock, create_msg(MOVE_CURSOR, user_id, b->id, b->ver++, cc));
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
			cc = malloc(2);
			cc[0]=c; cc[1]=0;
            send_msg(sock, create_msg(INSERT, user_id, b->id, b->ver++, cc));
            bcursor_insert(b, OWN_CURS_ID, c);
    }

	return quit;
}

int worker_loop(int sock)
{
	// log in to worker
	char *payload = create_login_payload("pisti", "degec");
	printf(payload);
    message *msg = create_msg(LOGIN, -1, -1, -1, payload);
    send_msg( sock, msg );

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

	close( sock );
	return 0;
}

int server_loop(int sock)
{
	int worker_port=0;
	int quit=0;
	char *payload = create_login_payload("pisti", "degec");
    message* msg = create_msg(LOGIN, -1, -1, -1, payload);
    send_msg( sock, msg );

	while( !quit )
	{
        //set poll list
        poll_list[0].fd = sock;
        poll_list[0].events = POLLIN;
        poll_list[1].fd = -1;
        poll_list[1].events = POLLIN;
        
        if( poll( poll_list, 2, -1 ) > 0 )
        {
            //handle server socket message
            if( poll_list[0].revents & POLLIN )
            {
				msg = recv_msg( sock );
				if( !msg )
				{
					//printf("Null msg received\n");
					continue;
				}

				switch( msg->type )
				{
					case MSG_OK:
						user_id = msg->user_id;
						printf("%s\n\nType the number of the file you would like to edit! \n", msg->payload);
						scanf("%d", &file_id);
						send_msg(sock, create_msg(FILE_REQUEST, user_id, file_id, -1, NULL));
						break;

					case FILE_RESPONSE:
						sscanf(msg->payload, "M%d", &worker_port);
						quit = 1;
						break;

					case MSG_FAILED:
						// panic
						exit(1);
						break;

					// TODO add file upload and deletion options
				}

				delete_msg( msg );
            }
        }
	}
		
	// log out from main server
	send_msg(sock, create_msg(QUIT, user_id, -1, -1, NULL));
	close(sock);
	return worker_port;
}

int main()
{

	// log in to main server
	int client_sock = setup_ssl_connect(hport++, SERVER_NAME, PPORT);
	int worker_port = server_loop(client_sock);
		
	if( !worker_port ) return 0;

	client_sock = setup_ssl_connect(hport++, SERVER_NAME, worker_port);
	worker_loop( client_sock );

    close( client_sock );
	return 0;
}
