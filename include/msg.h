#ifndef msg_h
#define msg_h

//type of message, the processing depends on it
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

//socket message
typedef struct message {
    MSG_TYPE type;
    int user_id;
    int file_id;
    int file_version;
    //value depends on msg type
    char *payload;
} message;

//serialize msg to the JSON string
//char* serialize_message( message* msg );

//deserilaize JSON string to msg
//message* deserialize_message( char* serialized_message );

//create msg with defined attributes
message* create_msg( MSG_TYPE type, int user_id, int file_id, int file_version, char* payload );

//free msg
void delete_msg( message *msg );

//send defined msg to defined socket
int send_msg( int socket, message* msg );

//send defined msg to defined socket without delete
int send_msg_without_delete( int socket, message* msg );

//receive msg by defined socket
message* recv_msg( int socket );

//print msg to stdin
void print_msg( message *msg );


#endif