#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <msg.h>
#include <buffer.h>


#define SERV_ADDR "127.0.0.1"
#define MAX_CLIENTS 30

#define HEIGHT 20
#define WIDTH 20

buffer* b;

int client_id;
int client_socket[30];


int handle_login(message* msg)
{
    //TODO:
    return 1;
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

int handle_msg(int socket, message* msg)
{
    if(msg == NULL)
    {
        printf("null msg\n");
        return 0;
    }
    print_msg(msg);
    
    int user_id;
    buffer* buf;
    char* payload; 
    CMOVE_DIR dir;
    switch(msg->type)
    {
        case MSG_FAILED:
            break;
        case MSG_OK:
            break;
        case LOGIN:
            user_id = handle_login(msg);
            if (user_id)
            {
                send_msg(socket, create_msg(MSG_OK, client_id, -1, NULL));
                client_id++;
                bcursor_new(b, client_id, 0, 0);
            } else
            {
                send_msg(socket, create_msg(MSG_FAILED, -1, -1, NULL));
            }
            break;
        case REGISTER:
            user_id = handle_register(msg);
            if (user_id)
            {
                send_msg(socket, create_msg(MSG_OK, user_id, -1, NULL));
            } else
            {
                send_msg(socket, create_msg(MSG_FAILED, -1, -1, NULL));
            }
            break;
        case QUIT:
            close(socket);
            break;
        case FILE_REQUEST:
            buf = search_file(msg->file_id);
            payload = buffer_serialize(buf);
            send_msg(socket, create_msg(FILE_RESPONSE, -1, msg->file_id, payload));
            break;
        case FILE_RESPONSE:
            break;
        case FILE_LIST:
            buf = search_file(msg->file_id);
            payload = buffer_serialize(buf);
            send_msg(socket, create_msg(FILE_RESPONSE, -1, msg->file_id, msg->payload));
            break;
        case DELETE_FILE:
            delete_file(msg->file_id);
            break;
        case INSERT:
            buf = search_file(msg->file_id);
            bcursor_insert(buf, msg->user_id, msg->payload[0]);
            send_msg_everyone(client_socket, 30, socket, msg);
            break;
        case INSERT_LINE:
            buf = search_file(msg->file_id);
            bcursor_insert_line(buf, msg->user_id);
            send_msg_everyone(client_socket, 30, socket, msg);
            break;
        case DELETE:
            buf = search_file(msg->file_id);
            bcursor_del(buf, msg->user_id);
            send_msg_everyone(client_socket, 30, socket, msg);
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
            buf = search_file(msg->file_id);
            bcursor_move(buf, msg->user_id, dir);
            send_msg_everyone(client_socket, 30, socket, msg);
            break;
        case ADD_CURSOR:
            buf = search_file(msg->file_id);
            bcursor_new(buf, msg->user_id, 0, 0);
            break;
        case DELETE_CURSOR:
            //TODO:
            break;
        default:
            break;
    }
    if(msg != NULL)
    {
        delete_msg(msg);
    }
    //TODO:
    return 1;
}


int main(void)
{
    struct sockaddr_in address;
    fd_set readfdset;
    int sd, maxfd;
    int server_socket , new_socket, activity, i;
    socklen_t addrlen;
    char buf[1024];
    int amount;
    
    client_id = 0;
    
    //create buffer
    b = buffer_new(0, 0, HEIGHT, WIDTH, 0);
    //b = buffer_from_file("testfile", 0, HEIGHT, WIDTH, 1);

    // create socket
    if((server_socket = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
    perror("socket");
    return -1;
    }

    //set server socket to allow multiple connections
    int opt = 1;
    if( setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        return -1;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    //address.sin_addr.s_addr = inet_addr(SERV_ADDR);
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( 8888 );
  
    addrlen = sizeof(address);

    //bind socket to the addres
    if(bind(server_socket, (struct sockaddr *)&address, addrlen) < 0)
    {
        perror("bind");
        return -1;
    }

    //set the server mode
    listen(server_socket, 5);

    for (i = 0; i < MAX_CLIENTS; i++) 
    {
        client_socket[i] = 0;
    }

    //wait the client
    while(1)
    {
        //clear the socket set
        FD_ZERO(&readfdset);
        //add server socket to set
        FD_SET(server_socket, &readfdset);
        maxfd = server_socket;
            
        //add client sockets to set
        for ( i = 0 ; i < MAX_CLIENTS ; i++) 
        {
            //socket descriptor
            sd = client_socket[i];
            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfdset);
            //highest file descriptor number, need it for the select function
            if(sd > maxfd)
                maxfd = sd;
        }
    
        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        activity = select( maxfd + 1 , &readfdset , NULL , NULL , NULL);
        if (activity < 0) 
        {
            printf("select error");
        }
            
        //If something happened on the server socket , then its an incoming connection
        if (FD_ISSET(server_socket, &readfdset)) 
        {
            if ((new_socket = accept(server_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
            {
                perror("accept");
            }
            
            //inform user of socket number - used in send and receive commands
            //printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
            printf("New connection\n");
                
            //add new socket to array of sockets
            for (i = 0; i < MAX_CLIENTS; i++) 
            {
                //if position is empty
                if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n" , i);
                    break;
                }
            }
        }
            
        //else its some IO operation or some other socket :)
        for (i = 0; i < MAX_CLIENTS; i++) 
        {
            sd = client_socket[i];
                
            if (FD_ISSET( sd , &readfdset)) 
            {
                message* msg = recv_msg(sd);
                if(msg)
                {
                    //printf("client message: %d\n", sd);
                    handle_msg(sd, msg);
                }
            }
        }
    }

    //close server socket
    close(server_socket);

    return 0;
    }
