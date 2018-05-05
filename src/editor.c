#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <poll.h>
#include <buffer.h>
#include <msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SERV_ADDR "127.0.0.1"
#define PORT 8887
#define KEY_ESC 27
#define OWN_CURS_ID 0

buffer* b;
int user_id;
int file_id = 1;
int file_version = 1;
int quit = 0;

//handle keyboard input
int handle_input(int server_socket)
{
    char payload[2];
    
    //read keyboard input
    int c = getch();
    
    switch(c){
        case KEY_LEFT:
            send_msg(server_socket, create_msg(MOVE_CURSOR, user_id, file_id, file_version, "2"));
            bcursor_move(b, OWN_CURS_ID, LEFT);
            break;
            
        case KEY_RIGHT:
            send_msg(server_socket, create_msg(MOVE_CURSOR, user_id, file_id, file_version, "3"));
            bcursor_move(b, OWN_CURS_ID, RIGHT);
            break;

        case KEY_UP:
            send_msg(server_socket, create_msg(MOVE_CURSOR, user_id, file_id, file_version, "0"));
            bcursor_move(b, OWN_CURS_ID, UP);
            break;

        case KEY_DOWN:
            send_msg(server_socket, create_msg(MOVE_CURSOR, user_id, file_id, file_version, "1"));
            bcursor_move(b, OWN_CURS_ID, DOWN);
            break;

        case KEY_BACKSPACE:
            send_msg(server_socket, create_msg(DELETE, user_id, file_id, file_version, NULL));
            bcursor_del(b, OWN_CURS_ID);
            break;
            
        case KEY_ESC:
            send_msg(server_socket, create_msg(QUIT, user_id, file_id, file_version, NULL));
            buffer_free(b);
            quit = 1;
            break;

        case KEY_F(8):
            /*buffer_save("atestfile", b);
            buffer_free(b);*/
            break;

        case KEY_F(7):
            /*data = buffer_serialize(b);
            buffer_free(b);
            b = buffer_deserialize(data, 1);
            free(data);*/
            break;

        case '\n':
            send_msg(server_socket, create_msg(INSERT_LINE, user_id, file_id, file_version, NULL));
            bcursor_insert_line(b, OWN_CURS_ID);
            break;

        default:
            payload[0] = c;
            payload[1] = 0;
            send_msg(server_socket, create_msg(INSERT, user_id, file_id, file_version, payload));
            bcursor_insert(b, OWN_CURS_ID, c);
    }
}

int handle_msg(int server_socket, message* msg)
{
    CMOVE_DIR dir;

    if(msg == NULL)
    {
        //printf("null msg\n");
        return 0;
    }

    switch(msg->type)
    {
        case MSG_OK:
            user_id = msg->user_id;
            send_msg(server_socket, create_msg(FILE_REQUEST, user_id, file_id, file_version, NULL));
            break;
        case FILE_RESPONSE:
            b = buffer_deserialize(msg->payload, 1);
            //bcursor_new(b, user_id, 0, 0);
            //b->u = ui_init(b);
            buffer_add_ui(b);
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
            if(strcmp(msg->payload, "0") == 0)
                dir = UP;
            if(strcmp(msg->payload, "1") == 0)
                dir = DOWN;
            if(strcmp(msg->payload, "2") == 0)
                dir = LEFT;
            if(strcmp(msg->payload, "3") == 0)
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

int main(void)
{
    size_t addrlen;
    struct sockaddr_in address;
    int server_socket;
    struct pollfd poll_list[2];


	if( fork() == 0 )
	{
		//setpgid(0,0);
		unlink("backpipe_c2s");
		mknod("backpipe_c2s", S_IFIFO | 0600, 0);
		//system("mknod backpipe_c2s p");
		char cmd[1024];
		// for client
		snprintf(cmd, 1024, "nc --ssl --ssl-verify --ssl-trustfile b.crt localhost 4444 0<backpipe_c2s  | nc -l %d -k > backpipe_c2s", PORT);
		// for server
		//snprintf(cmd, 1024, "nc --ssl --ssl-cert b.crt --ssl-key b.key -l 4444 0<backpipe_s2c | nc %s %d | tee backpipe_s2c", SERV_ADDR, PORT);
		system(cmd);
		return 0;
	}
	sleep(1);
	
    
    //create socket
    if( ( server_socket = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
        perror("socket");
        return -1;
    }
    
    //set family, address, port
    memset( &address, 0, sizeof( address ) );
    address.sin_addr.s_addr = inet_addr( SERV_ADDR );
    address.sin_family = AF_INET;
    address.sin_port = htons( PORT );
    //count address length
    addrlen = sizeof( address );
    
    //connect to server
    if( connect( server_socket, (struct sockaddr *)&address, addrlen ) < 0 )
    {
        perror( "connect" );
        return -1;
    }
    
    
    //login to server
    /*char username[20];
    char password[20];
    printf("username: ");
    scanf("%s", &username);
    printf("password: ");
    scanf("%s", &password);*/
    
    //login message
    //printf("sending login msg...\n");
    message* msg = create_msg( LOGIN, user_id, -1, -1, "login" );
    send_msg( server_socket, msg );
    //printf("send login msg\n");
        
    
    while( quit == 0 )
    {
        //set poll list
        poll_list[0].fd = server_socket;
        poll_list[0].events = POLLIN;
        poll_list[1].fd = STDIN_FILENO;
        poll_list[1].events = POLLIN;
        
        if( poll( poll_list, 2, -1 ) > 0 )
        {
            //handle server socket message
            if( poll_list[0].revents & POLLIN )
            {
                //handle message
                handle_msg(server_socket, recv_msg(server_socket));
            }
            //handle stdin input
            if( poll_list[1].revents & POLLIN )
            {
                //handle keyboard input
                handle_input(server_socket);
            }
            
        }
    }
    
    //close socket
    close( server_socket );
    //end curses mode  
    endwin();  
    
    return 0;
}
