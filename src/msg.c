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
    //send(socket, serialized_msg, strlen(serialized_msg), 0);
    write(socket, serialized_msg, strlen(serialized_msg));
    delete_msg(msg);
}

message* recv_msg(int socket)
{
    char* serialized_msg;
    //int amount = recv(socket, serialized_msg, 1024, 0);
    int amount = read(socket, serialized_msg, 1024);
    if(amount <= 0)
        return NULL;
    message* msg = deserialize_msg(serialized_msg);
    return msg;
}
