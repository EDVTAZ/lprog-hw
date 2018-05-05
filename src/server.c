#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <poll.h>
#include <msg.h>
#include <buffer.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define SERV_ADDR "127.0.0.1"
#define PORT 8889
#define PPORT 8888
#define MAX_CLIENTS 30
#define HEIGHT 20
#define WIDTH 20
#define CERT "b.crt"
#define KEY "b.key"

buffer* b;

int client_id;
struct pollfd poll_list[MAX_CLIENTS + 1];

//send msg to every relevant clients
int send_msg_everyone( int sender_socket, message* msg )
{
    int i;
    for( i = 1; i <= MAX_CLIENTS; i++ )
    {
        if( poll_list[i].fd >= 0 && poll_list[i].fd != sender_socket )
        {
            send_msg_without_delete( poll_list[i].fd, msg );
        }
    }
    
}

int handle_login(message* msg)
{
    client_id++;
    return client_id;
}

int handle_register(message* msg)
{
    //TODO:
    return 1;
}

int delete_file()
{
    //TODO:
    return 1;
}

buffer* search_file(int file_id)
{
    return b;
}

int handle_msg( int socket, message* msg )
{
    if(msg == NULL)
    {
        printf("null msg\n");
        return 0;
    }
    
    print_msg(msg);
    
    int i, user_id;
    buffer* buf;
    char* payload; 
    CMOVE_DIR dir;
    
    switch( msg->type )
    {
        //handle sign in
        case LOGIN:
            user_id = handle_login(msg);
            if (user_id)
            {
                send_msg(socket, create_msg(MSG_OK, user_id, -1, -1, NULL));
            }else
            {
                send_msg(socket, create_msg(MSG_FAILED, -1, -1, -1, NULL));
            }
            break;
        
        //handel sign up
        case REGISTER:
            user_id = handle_register(msg);
            if (user_id)
            {
                send_msg(socket, create_msg(MSG_OK, user_id, -1, -1, NULL));
            }else
            {
                send_msg(socket, create_msg(MSG_FAILED, -1, -1, -1, NULL));
            }
            break;
            
        //handle logout
        case QUIT:
            //delete cursor from buffer
            buf = search_file(msg->file_id);
            bcursor_free(buf, msg->user_id);
            
            //delete cursor everywhere
            send_msg_everyone(socket, create_msg(DELETE_CURSOR, msg->user_id, msg->file_id, msg->file_version, NULL));
            
            //delete from poll list
            for (i = 1; i <= MAX_CLIENTS; i++) 
            {
                if(poll_list[i].fd == socket)
                {
                    poll_list[i].fd = -1;
                    break;
                }
            }
            
            //close client socket
            //close(socket);
            break;
        
        //reply the requested file
        case FILE_REQUEST:
            buf = search_file(msg->file_id);
            payload = buffer_serialize(buf);
			printf("%s\n", payload);
            send_msg(socket, create_msg(FILE_RESPONSE, -1, msg->file_id, msg->file_version, payload));
            bcursor_copy_own(buf, user_id);
            send_msg_everyone(socket, create_msg(ADD_CURSOR, user_id, msg->file_id, msg->file_version, NULL));
            break;
            
        //delete the defined file from server
        case DELETE_FILE:
            delete_file(msg->file_id);
            break;
            
        //insert character to buffer and send everyone
        case INSERT:
            buf = search_file(msg->file_id);
            bcursor_insert(buf, msg->user_id, msg->payload[0]);
            send_msg_everyone(socket, msg);
            break;
            
        //insert new line to buffer and send everyone
        case INSERT_LINE:
            buf = search_file(msg->file_id);
            bcursor_insert_line(buf, msg->user_id);
            send_msg_everyone(socket, msg);
            break;
            
        //delete character from buffer and send everyone
        case DELETE:
            buf = search_file(msg->file_id);
            bcursor_del(buf, msg->user_id);
            send_msg_everyone(socket, msg);
            break;
            
        //move cursor in buffer and send everyone
        case MOVE_CURSOR:
            if(strcmp(msg->payload, "0") == 0)
                dir = UP;
            if(strcmp(msg->payload, "1") == 0)
                dir = DOWN;
            if(strcmp(msg->payload, "2") == 0)
                dir = LEFT;
            if(strcmp(msg->payload, "3") == 0)
                dir = RIGHT;
            buf = search_file(msg->file_id);
            bcursor_move(buf, msg->user_id, dir);
            send_msg_everyone(socket, msg);
            break;
            
        //add cursor to buffer and send everyone
        case ADD_CURSOR:
            buf = search_file(msg->file_id);
            bcursor_new(buf, msg->user_id, 0, 0);
            break;
            
        //delete cursor from buffer and send everyone
        case DELETE_CURSOR:
            //TODO:
            break;
            
        //default case
        default:
            break;
    }
    
    //free msg
    delete_msg(msg);
    
    //TODO:
    return 1;
}


int main( void )
{
    socklen_t addrlen;
    struct sockaddr_in address;
    int i, server_socket , new_socket;
    
    client_id = 0;
    
    //create buffer
    b = buffer_new( 0, 0, HEIGHT, WIDTH, 0 );
    //b = buffer_from_file("testfile", 0, HEIGHT, WIDTH, 1);

    // create server socket
    if( ( server_socket = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
    perror( "socket" );
    return -1;
    }

    //set server socket to allow multiple connections
    int opt = 1;
    if( setsockopt( server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof( opt ) ) < 0 )
    {
        perror( "setsockopt" );
        return -1;
    }

    //set family, address, port
    memset( &address, 0, sizeof( address ) );
    address.sin_family = AF_INET;
    //address.sin_addr.s_addr = inet_addr(SERV_ADDR);
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    //count address length
    addrlen = sizeof(address);

    //bind socket to the addres
    if(bind(server_socket, (struct sockaddr *)&address, addrlen) < 0)
    {
        perror( "bind" );
        return -1;
    }

    //set the server mode
    listen( server_socket, 5 );

    //set poll list
    poll_list[0].fd = server_socket;
    poll_list[0].events = POLLIN;
    for ( i = 1; i <= MAX_CLIENTS; i++ ) 
    {
        poll_list[i].fd = -1;
        poll_list[i].events = POLLIN;
    }

	if( fork() == 0 )
	{
		////setpgid(0,0);
		//unlink("backpipe_s2c");
		//mknod("backpipe_s2c", S_IFIFO | 0600, 0);
		////system("mknod backpipe_s2c p");
		//char cmd[1024];
		//// for client
		////snprintf(cmd, 1024, "nc --ssl --ssl-verify --ssl-trustfile b.crt localhost 4444 0<backpipe_c2s | nc -l %d > backpipe_c2s", PORT);
		//// for server
		//snprintf(cmd, 1024, "nc --ssl --ssl-cert b.crt --ssl-key b.key -l 4444 -k 0<backpipe_s2c  | nc localhost %d > backpipe_s2c", PORT);
		char cmd[1024];
		snprintf(cmd, 1024, "socat openssl-listen:%d,cert=%s,key=%s,verify=0,reuseaddr,fork tcp4:localhost:%d", PPORT, CERT, KEY, PORT);
		system(cmd);
		return 0;
	}

    //wait the client
    while( 1 )
    {
        if( poll( poll_list, MAX_CLIENTS + 1, -1 ) > 0 )
        {
            //handle incoming new connection
            if( poll_list[0].revents & POLLIN )
            {
                printf( "New connection\n" );
                if( ( new_socket = accept( server_socket, NULL, NULL ) ) < 0 )
                {
                    perror( "accept" );
                }
                
                //add fd to poll list
                for( i = 1; i <= MAX_CLIENTS; i++ )
                {
                    if(poll_list[i].fd < 0)
                    {
                        poll_list[i].fd = new_socket;
                        printf( "Adding to list of sockets as %d\n" , i );
                        break;
                    }
                }
            }
            //handle socket events
            for( i = 1; i <= MAX_CLIENTS; i++ )
            {
                //handle socket error and socket close
                if( poll_list[i].revents & (POLLERR | POLLHUP) )
                {
                    printf( "connection closed: %d", poll_list[i].fd );
                    close( poll_list[i].fd );
                    poll_list[i].fd = -1;
                }
                //handle incoming message
                else if( poll_list[i].revents & POLLIN )
                {
                    message* msg = recv_msg( poll_list[i].fd );
                    if( msg )
                    {
                        handle_msg( poll_list[i].fd, msg );
                    }
                }
            }
        }
    }

    //close server socket
    close( server_socket );

    return 0;
}
