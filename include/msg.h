#ifndef msg_h
#define msg_h

//type of message, the processing depends on it
typedef enum msg_type {
    MSG_FAILED, 
    MSG_OK,
    LOGIN,
    REGISTER, 
    QUIT,
    FILE_REQUEST,
    FILE_RESPONSE,
    FILE_LIST,
    CREATE_FILE,
    DELETE_FILE, 
    INSERT,
    INSERT_LINE,
    DELETE,
    MOVE_CURSOR,
    ADD_CURSOR,
    DELETE_CURSOR
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