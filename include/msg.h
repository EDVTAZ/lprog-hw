#ifndef msg_h
#define msg_h

typedef enum msg_type {
    MSG_FAILED = 100, 
    MSG_OK = 0,
    LOGIN = 1,
    REGISTER = 2, 
    QUIT = 3,
    FILE_REQUEST = 10,
    FILE_RESPONSE = 11,
    FILE_LIST = 12,
    DELETE_FILE = 13, 
    INSERT = 20,
    INSERT_LINE =  21,
    DELETE = 22,
    MOVE_CURSOR = 23,
    ADD_CURSOR = 24,
    DELETE_CURSOR = 25
} MSG_TYPE;

typedef struct message {
    MSG_TYPE type;
    int user_id;
    int file_id;
    char *payload;
} message;

int send_msg(int socket, message* msg);

int send_msg_everyone(int sockets[], int size, int sender_socket, message* msg);

message* recv_msg(int socket);

char* serialize_message(message* msg);

message* deserialize_message(char* serialized_message);

void delete_msg(message *msg);

message* create_msg(MSG_TYPE type, int user_id, int file_id, char* payload);

void print_msg(message *msg);


#endif