#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <buffer.h>
#include <msg.h>

#define SERV_ADDR "127.0.0.1"
#define PORT 8888
//asdsad
buffer* b;
int user_id;
int file_id = 1;

int handle_input(int server_socket)
{
    message* msg;
    char payload[2];
    int c = getch();
    
    switch(c){
        case KEY_LEFT:
            msg = create_msg(MOVE_CURSOR, user_id, file_id, "2");
            send_msg(server_socket, msg);
            bcursor_move(b, user_id, LEFT);
            break;
            
        case KEY_RIGHT:
            msg = create_msg(MOVE_CURSOR, user_id, file_id, "3");
            send_msg(server_socket, msg);
            bcursor_move(b, user_id, RIGHT);
            break;

        case KEY_UP:
            msg = create_msg(MOVE_CURSOR, user_id, file_id, "0");
            send_msg(server_socket, msg);
            bcursor_move(b, user_id, UP);
            break;

        case KEY_DOWN:
            msg = create_msg(MOVE_CURSOR, user_id, file_id, "1");
            send_msg(server_socket, msg);
            bcursor_move(b, user_id, DOWN);
            break;

        case KEY_BACKSPACE:
            msg = create_msg(DELETE, user_id, file_id, NULL);
            send_msg(server_socket, msg);
            bcursor_del(b, user_id);
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
            msg = create_msg(INSERT_LINE, user_id, file_id, NULL);
            send_msg(server_socket, msg);
            bcursor_insert_line(b, user_id);
            break;

        default:
            payload[0] = c;
            payload[1] = 0;
            msg = create_msg(INSERT, user_id, file_id, payload);
            send_msg(server_socket, msg);
            bcursor_insert(b, user_id, c);
    }
}

int handle_msg(int server_socket, message* msg)
{
    //if(b == NULL) printf("server msg\n");
    //print_msg(msg);
    CMOVE_DIR dir;
    message* delmsg = msg;
    switch(msg->type)
    {
        case MSG_OK:
            user_id = msg->user_id;
            msg = create_msg(FILE_REQUEST, user_id, file_id, NULL);
            send_msg(server_socket, msg);
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
            if(user_id != msg->user_id) bcursor_new(b, msg->user_id, 0, 0);
            break;
        case DELETE_CURSOR:
            //TODO:
            break;
        default:
            break;
            
    }
    delete_msg(delmsg);
}

int main(void)
{
    fd_set readfdset;
    int server_socket, maxfd, activity;
    struct sockaddr_in address;
    size_t addrlen;
    char buf[1024];
    int amount;
    
    //create socket
    if((server_socket = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return -1;
    }
    
    memset(&address, 0, sizeof(address));
    address.sin_addr.s_addr = inet_addr( SERV_ADDR );
    address.sin_family = AF_INET;
    address.sin_port = htons( PORT );
    addrlen = sizeof(address);
    
    //connect to server
    if(connect(server_socket, (struct sockaddr *)&address, addrlen) < 0)
    {
        perror("connect");
        return -1;
    }
    
    message* msg;
    
    //login to server
    /*char username[20];
    char password[20];
    printf("username: ");
    scanf("%s", &username);
    printf("password: ");
    scanf("%s", &password);*/
    
    //login message
    printf("sending login msg...\n");
    msg = create_msg(LOGIN, user_id, -1, "asdas");
    send_msg(server_socket, msg);
    printf("send login msg\n");
        
    
    while(1)
    {
        //clear the socket set
        FD_ZERO(&readfdset);
        //add server socket and standard input to set
        FD_SET(server_socket, &readfdset);
        FD_SET(STDIN_FILENO, &readfdset);
        if(server_socket > 0) FD_SET(server_socket, &readfdset);
        //set maxfd
        maxfd = server_socket > STDIN_FILENO ? server_socket : STDIN_FILENO;
        
        activity = select( maxfd + 1 , &readfdset , NULL , NULL , NULL);
        if (activity < 0) 
        {
            perror("select");
        }
            
        if (FD_ISSET(server_socket, &readfdset)) 
        {
            //handle message
            handle_msg(server_socket, recv_msg(server_socket));
        }
        if (FD_ISSET(STDIN_FILENO, &readfdset)) 
        {
            //handle message
            handle_input(server_socket);
        }
        
    }
    
    //close socket
    close(server_socket);
    //end curses mode  
    endwin();  
    
    return 0;
}








/*#include <ncurses.h>
#include <rope.h>
#include <stdlib.h>

#include <buffer.h>
#include <ui.h>

#define HEIGHT 20
#define WIDTH 20

int main()
{


    //buffer* b = buffer_new(0, 0, HEIGHT, WIDTH, 1);
    buffer* b = buffer_from_file("testfile", 0, HEIGHT, WIDTH, 1);
    bcursor_new(b, 1, 0, 0);
    char* data;
    int size;

    int cid = 0;

    ui_update(b->u);

    int c;
    while(c != KEY_F(8)){
        cid ^= 1;
        c = getch();

        switch(c){
            case KEY_LEFT:
                bcursor_move(b, cid, LEFT);
                break;
                
            case KEY_RIGHT:
                bcursor_move(b, cid, RIGHT);
                break;

            case KEY_UP:
                bcursor_move(b, cid, UP);
                break;

            case KEY_DOWN:
                bcursor_move(b, cid, DOWN);
                break;

            case KEY_BACKSPACE:
                bcursor_del(b, cid);
                break;

            case KEY_F(8):
                buffer_save("atestfile", b);
                buffer_free(b);
                break;

            case KEY_F(7):

                data = buffer_serialize(b);

                buffer_free(b);

                b = buffer_deserialize(data, 1);
                free(data);

                break;

            case '\n':
                bcursor_insert_line(b, cid);
                break;

            default:
                bcursor_insert(b, cid, c);
        }
    }


    endwin();


    return 0;
}*/
