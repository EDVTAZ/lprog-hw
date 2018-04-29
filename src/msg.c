#include <msg.h>
#include <parson.h>

char* serialize_msg(message* msg)
{
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;

    // serialized the attributes
    json_object_set_number(root_object, "type", msg->type);
    json_object_set_number(root_object, "user_id", msg->user_id);
    json_object_set_number(root_object, "file_id", msg->file_id);
    if(msg->payload)
        json_object_set_string(root_object, "payload", msg->payload);
    
    serialized_string = json_serialize_to_string(root_value);
    json_value_free(root_value);
    return serialized_string;
}

message* deserialize_msg(char* serialized_msg)
{
    JSON_Value *root_value = json_parse_string(serialized_msg);
    JSON_Object *root_object = json_value_get_object(root_value);
    message* msg = malloc(sizeof(msg));
    
    //deserialized the attributes
    msg->type = (int)json_object_get_number(root_object, "type");
    msg->user_id = (int)json_object_get_number(root_object, "user_id");
    msg->file_id = (int)json_object_get_number(root_object, "file_id");
    
    if(json_object_has_value(root_object, "payload"))
        msg->payload = (char*)json_object_get_string(root_object, "payload");
    else
        msg->payload = NULL;

    return msg;
    
}

void delete_msg(message* msg)
{
    if(msg)
    {
        free(msg);
    }
}

message* create_msg(MSG_TYPE type, int user_id, int file_id, char* payload)
{
    message* msg = malloc(sizeof(msg));
    msg->type = type;
    msg->user_id = user_id;
    msg->file_id = file_id;
    msg->payload = payload;
    return msg;
}

int send_msg(int socket, message* msg)
{
    char* serialized_msg = serialize_msg(msg);
    send(socket, serialized_msg, strlen(serialized_msg), 0);
    //write(socket, serialized_msg, strlen(serialized_msg));
    delete_msg(msg);
}

int send_msg_everyone(int sockets[], int size, int sender_socket, message* msg)
{
    int i;
    char* serialized_msg = serialize_msg(msg);
    for(i = 0; i < size; i++)
    {
        if(sockets[i] != 0 && sockets[i] != sender_socket)
        {
            send(sockets[i], serialized_msg, strlen(serialized_msg), 0);
        }
    }
    //write(socket, serialized_msg, strlen(serialized_msg));
    //delete_msg(msg);
}

message* recv_msg(int socket)
{
    char serialized_msg[2048];
    int amount = recv(socket, serialized_msg, 2048, 0);
    //int amount = read(socket, serialized_msg, 2048);
    if(amount <= 0)
        return NULL;
    message* msg = deserialize_msg(serialized_msg);
    return msg;
}

void print_msg(message *msg)
{
    printf("MSG:\n");
    printf("\ttype:\t");
    switch(msg->type)
    {
        case MSG_FAILED:
            printf("MSG_FAILED\n");
            break;
        case MSG_OK:
             printf("MSG_OK\n");
            break;
        case LOGIN:
             printf("LOGIN\n");
            break;
        case REGISTER:
             printf("REGISTER\n");
            break;
        case QUIT:
             printf("QUIT\n");
            break;
        case FILE_REQUEST:
             printf("FILE_REQUEST\n");
            break;
        case FILE_RESPONSE:
             printf("FILE_RESPONSE\n");
            break;
        case FILE_LIST:
             printf("FILE_LIST\n");
            break;
        case DELETE_FILE:
             printf("DELETE_FILE\n");
            break;
        case INSERT:
             printf("INSERT\n");
            break;
        case INSERT_LINE:
             printf("INSERT_LINE\n");
            break;
        case DELETE:
             printf("DELETE\n");
            break;
        case MOVE_CURSOR:
             printf("MOVE_CURSOR\n");
            break;
        case ADD_CURSOR:
             printf("ADD_CURSOR\n");
            break;
        case DELETE_CURSOR:
             printf("DELETE_CURSOR\n");
            break;
        default:
            printf("-\n");
            break;
    }
    printf("\tuser_id:\t%d\n", msg->user_id);
    printf("\tfile_id:\t%d\n", msg->file_id);
    printf("\tpayload:\t%s\n", msg->payload);
    printf("MSG_END!\n");
}
