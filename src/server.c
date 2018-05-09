#include <buffer.h>
#include <msg.h>

#include <parson.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <poll.h>
#include <fcntl.h>

#define PPORT 8888
#define MAX_CLIENTS 5
#define MAX_WORKERS 5
#define MAX 64
#define CERT "b.crt"
#define KEY "b.key"
#define HEIGHT 20
#define WIDTH 20

enum {SERVER_MODE, WORKER_MODE} mode;
int hport = 46957;

struct pollfd poll_list[MAX_CLIENTS+1];
int clients[MAX_CLIENTS+1] = {0};
#define REGISTERED_CLIENTS 3
char *names[] = {"balzs", "gbor", "pisti"};
char *passes[] = {"password", "12345", "degec"};

int worker_ports[MAX_WORKERS+1];
#define MANAGED_FILE_NO 3
char *files[] = {"", "testfile0", "testfile1", "testfile2"};
buffer* buf;

//replace \n to 0
char* terminateNull(char* s){
    int i = 0;
    while(1){
        if(s[i] == 0) break;
        if(s[i] == '\n')
        {
            s[i] = 0;
            break;
        }
        i++;
    }
    return s;
}

//username and password is equivalent
int is_equivalent_strings(const char* a1, const char* a2, const char* b1, const char* b2){
    if(strcmp(a1, a2) == 0 && strcmp(b1, b2) == 0){
        return 1;
    }
    return 0;
}

//if valid user data return user_id else -1
int validate_user(char *payload)
{
    JSON_Value *root_value = json_parse_string(payload);
    JSON_Object *root_object = json_value_get_object(root_value);

	const char *name = json_object_get_string(root_object, "name");
	const char *pass = json_object_get_string(root_object, "pass");

        char* username;
        char* password;
        char * line = NULL;
        size_t len = 0;
        ssize_t read;
        
        //open file
        FILE * fp = fopen("userdata.txt", "r");
        if (fp == NULL){
            perror("userdata open");
            json_value_free(root_value);
            return -1;
        }
        
        int user_id = 1;
        int val = 0;
        //read userdata
        while ((read = getline(&line, &len, fp)) != -1) {
            username = strtok (line,":");
            password = strtok (NULL, ":");
            val = (int) is_equivalent_strings(name, username, pass, terminateNull(password));
            if(val) break;
            user_id++;
        }
        //free and close
        json_value_free(root_value);
        fclose(fp);
        //return
        if(val)
		{
			clients[user_id] = 1;
			return user_id;
		}
        return -1;
}

//if successful register return user_id else -1
int validate_register(char* payload){
    
    JSON_Value *root_value = json_parse_string(payload);
    JSON_Object *root_object = json_value_get_object(root_value);

    const char *name = json_object_get_string(root_object, "name");
    const char *pass = json_object_get_string(root_object, "pass");
    
    char* username;
    char* password;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    
    //open file
    FILE * fp = fopen("userdata.txt", "ab+");
    if (fp == NULL){
        json_value_free(root_value);
        perror("userdata open");
        return -1;
    }
    
    int user_id = 1;
    int val = 0;
    //read userdata
    while ((read = getline(&line, &len, fp)) != -1) {
        username = strtok (line,":");
        val = is_equivalent_strings(name, username, "", "");
        if(val) break;
        user_id++;
    }
    //return if exist same username
    if(val) {
        json_value_free(root_value);
        fclose(fp);
        return -1;
        
    }
    //write userdata
    if(user_id == 1){
        fprintf(fp, "%s:%s", name, pass);
    }else{
        fprintf(fp, "\n%s:%s", name, pass);
    }
    //close, free and return
    json_value_free(root_value);
    fclose(fp);
    return user_id;
}

//if successful create return file id else -1
//create + write in files
int createFile(char* filename){
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    
    //open file
    FILE * fp = fopen("files.txt", "ab+");
    if (fp == NULL){
        perror("files open");
        return -1;
    }
    //create file
    FILE* file_ptr = fopen(filename, "w");
    if (file_ptr == NULL){
        perror("create file");
        fclose(fp);
        return -1;
    }
    fclose(file_ptr);
    
    int file_id = 1;
    int val = 0;
    //read files
    while ((read = getline(&line, &len, fp)) != -1) {
        val = is_equivalent_strings(filename, line, "", "");
        if(val) break;
        file_id++;
    }
    //if exist same filename
    if(val) {
        fclose(fp);
        return -1;
        
    }
    //write to files
    if(file_id == 1){
        fprintf(fp, "%s", filename);
    }else{
        fprintf(fp, "\n%s", filename);
    }
    //close
    fclose(fp);
    return file_id;
}

//TODO:
int deleteFile(int file_id){
    
}

//return filename
char* getFileName(int file_id){
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    
    //open file
    FILE * fp = fopen("files.txt", "r");
    if (fp == NULL){
        perror("files open");
    }
    //read and search filename
    int i = 1;    
    while ((read = getline(&line, &len, fp)) != -1) {
        if(i == file_id){
            fclose(fp);
            return terminateNull(line);
        }
        i++;
    }
    //close and return
    fclose(fp);
    return NULL;
}

//add user data to file_id
void addFileToUser(int file_id, int user_id){
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    
    //open file
    FILE * fp = fopen("userdata.txt", "r+");
    if (fp == NULL){
        perror("userdata open");
    }
    //TODO: now overwrite data 
    
    int i = 1;
    //read userdata
    while ((read = getline(&line, &len, fp)) != -1) {
        if(i == user_id) {
            fseek(fp, -1, SEEK_CUR);
            //write
            fprintf(fp, ":%d\n", file_id);
            break;
        }
        i++;
    }
    //close
    fclose(fp);
}

//TODO
int logged_in(int user_id)
{
	return clients[user_id];
}

int setup_ssl_listen(int local_port, int public_port)
{
	// starts listening through socat ssl tunnel 
	// returns with socket fd
	// local port is the one that our program listens on and public port is the one socat ssl listens on
	// if either of the ports are unavailable, it tries to increment it to find one that is
	//
	int sock;
    socklen_t addrlen;
    struct sockaddr_in address;
    
    // create server socket
    if( (sock=socket( AF_INET, SOCK_STREAM, 0 )) < 0 )
    {
		perror( "socket" );
		return -1;
    }

    //set server socket to allow multiple connections
    int opt = 1;
    if( setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof( opt )) < 0 )
    {
        perror( "setsockopt" );
        return -1;
    }

    //set family, address, port
    memset( &address, 0, sizeof( address ) );
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( local_port );
    //count address length
    addrlen = sizeof(address);

    //bind socket to the addres
    if(bind(sock, (struct sockaddr *)&address, addrlen) < 0)
    {
        perror( "bind" );
        return -1;
    }

	//
	if(!listen(sock, 5))
	{
		perror("error starting listening");
		//return -1;
	}
    //set poll list
    poll_list[0].fd = sock;
    poll_list[0].events = POLLIN;
    for( int i=1; i<=MAX_CLIENTS; i++ ) 
    {
        poll_list[i].fd = -1;
        poll_list[i].events = POLLIN;
    }

	if( fork() == 0 )
	{
		char cmd[1024];
		snprintf(cmd, 1024, "socat openssl-listen:%d,cert=%s,key=%s,verify=0,reuseaddr,fork tcp4:localhost:%d", public_port, CERT, KEY, local_port);
		system(cmd);
		return 0;
	}

	return sock;
}

void close_socket(int socket)
{
	//delete from poll list
	for( int i=1; i<=MAX_CLIENTS; i++ ) 
	{
		if(poll_list[i].fd == socket)
		{
			poll_list[i].fd = -1;
			break;
		}
	}
	
	//close client socket
	close(socket);
}

int handle_msg( int socket, message* msg );
int server_listen(int sock)
{
	int new_socket;
    while(1)
    {
        if( poll( poll_list, MAX_CLIENTS+1, -1 ) > 0 )
        {
            //handle incoming new connection
            if( poll_list[0].revents & POLLIN )
            {
                printf( "New connection\n" );
                if( ( new_socket = accept( sock, NULL, NULL ) ) < 0 )
                {
                    perror( "accept" );
                }
                
                //add fd to poll list
                for( int i=1; i<=MAX_CLIENTS; i++ )
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
            for( int i=1; i<=MAX_CLIENTS; i++ )
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

	exit(0);
}

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

int get_worker(int file_id)
{
	// creates a worker for the file with file_id on the specified ports
	// fork is called inside, returns with the public port of the worker that clients should connect to
	// if worker for the file already exists this doesn't do anything (except return the correct port)

	if( worker_ports[file_id] ) return worker_ports[file_id];

	// TODO test if the given ports are free to use
	int lp = hport++;
	int pp = hport++;

	if( fork()==0 )
	{
		mode = WORKER_MODE;

		// set everyone logged out and empty sockets poll list
		for(int i=0; i<MAX_CLIENTS+1; i++)
		{
			clients[i] = 0;
			poll_list[i].fd = -1;
		}

		// init buffer
		//buf = buffer_from_file( files[file_id], file_id, HEIGHT, WIDTH, 0);
                buf = buffer_from_file( getFileName(file_id), file_id, HEIGHT, WIDTH, 0);
                
		// start listening
		int worker_sock = setup_ssl_listen(lp, pp);
		server_listen(worker_sock);

		// we get here if server is closing
		close(worker_sock);
		exit(0);
	}

	worker_ports[file_id] = pp;
	return pp;
}

int handle_msg( int socket, message* msg )
{
    if(msg == NULL)
    {
        printf("null msg\n");
        return 0;
    }
    
    print_msg(msg);
    
    char* payload; 
	int user_id;
	int port;
    CMOVE_DIR dir;

	if( !logged_in(msg->user_id) && msg->type != LOGIN && msg->type != REGISTER )
	{
		printf("User not logged in: %d", msg->user_id);
		close_socket(socket);
		return 1;
	}


	if(mode == SERVER_MODE)
	{
		switch( msg->type )
		{
			//handle sign in
			case LOGIN:
				//printf("%s ==\n", msg->payload);
				user_id = validate_user(msg->payload);
				if (user_id)
				{
                                        //TODO:
					char *pretty_file_list = malloc( 1024 );
					//snprintf(pretty_file_list, 1024, "1 - %s\n2 - %s\n3 - %s", files[1], files[2], files[3]);
                                        snprintf(pretty_file_list, 1024, "1 - %s\n2 - %s\n3 - %s", getFileName(1), getFileName(2), getFileName(3));
					send_msg(socket, create_msg(MSG_OK, user_id, -1, -1, pretty_file_list));
				}
				else
				{
					send_msg(socket, create_msg(MSG_FAILED, -1, -1, -1, NULL));
				}
				break;
			
			//handle sign up
			case REGISTER:
				user_id = validate_register(msg->payload);
				if (user_id)
				{
					send_msg(socket, create_msg(MSG_OK, user_id, -1, -1, NULL));
				}else
				{
					send_msg(socket, create_msg(MSG_FAILED, -1, -1, -1, NULL));
				}
				break;
				
			//reply the requested file
			case FILE_REQUEST:
				fflush(stdout);
				port = get_worker(msg->file_id);
				
				payload = malloc(24);
				snprintf(payload, 24, "M%d", port);
				printf("%s\n", payload);
				send_msg(socket, create_msg(FILE_RESPONSE, -1, msg->file_id, msg->file_version, payload));
				break;
				
			case CREATE_FILE:
					//TODO
					//file_id = (msg->payload);
					break;

			//delete the defined file from server
			case DELETE_FILE:
				// TODO !!
				break;
				
			//handle logout
			case QUIT:
				clients[msg->user_id] = 0;
				close_socket(socket);
				break;
				
			//default case
			default:
				break;
		}
	}
    
	if(mode == WORKER_MODE)
	{
		switch( msg->type )
		{
			//handle sign in
			case LOGIN:
				user_id = validate_user(msg->payload);
				if (user_id)
				{
					send_msg(socket, create_msg(MSG_OK, user_id, -1, -1, NULL));
				}
				else
				{
					send_msg(socket, create_msg(MSG_FAILED, -1, -1, -1, NULL));
				}
				break;

			//reply the requested file
			case FILE_REQUEST:
				payload = buffer_serialize(buf);
				printf("%s\n", payload);
				send_msg(socket, create_msg(FILE_RESPONSE, -1, msg->file_id, msg->file_version, payload));
				bcursor_copy_own(buf, user_id);
				send_msg_everyone(socket, create_msg(ADD_CURSOR, user_id, msg->file_id, msg->file_version, NULL));
				break;
			
			//handle logout
			case QUIT:
				//delete cursor from buffer
				bcursor_free(buf, msg->user_id);
				
				//delete cursor everywhere
				send_msg_everyone(socket, create_msg(DELETE_CURSOR, msg->user_id, msg->file_id, msg->file_version, NULL));
				
				close_socket(socket);
				break;
			
			//insert character to buffer and send everyone
			case INSERT:
				bcursor_insert(buf, msg->user_id, msg->payload[0]);
				send_msg_everyone(socket, msg);
				break;
				
			//insert new line to buffer and send everyone
			case INSERT_LINE:
				bcursor_insert_line(buf, msg->user_id);
				send_msg_everyone(socket, msg);
				break;
				
			//delete character from buffer and send everyone
			case DELETE:
				bcursor_del(buf, msg->user_id);
				send_msg_everyone(socket, msg);
				break;
				
			//move cursor in buffer and send everyone
			case MOVE_CURSOR:
				if(msg->payload[0] == '0')
					dir = UP;
				if(msg->payload[0] == '1')
					dir = DOWN;
				if(msg->payload[0] == '2')
					dir = LEFT;
				if(msg->payload[0] == '3')
					dir = RIGHT;
				bcursor_move(buf, msg->user_id, dir);
				send_msg_everyone(socket, msg);
				break;
				
			//default case
			default:
				break;
		}
	}
    
    //free msg
    delete_msg(msg);
    
    return 0;
}

int main()
{
	mode = SERVER_MODE;
	int server_sock = setup_ssl_listen(hport++, PPORT);
	server_listen(server_sock);

    close( server_sock );
	return 0;
}

