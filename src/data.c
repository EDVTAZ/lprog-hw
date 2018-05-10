#include <data.h>
#include <parson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//replace \n to 0
char* terminateNull(char* s)
{
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
int is_equivalent_strings(const char* a1, const char* a2, const char* b1, const char* b2)
{
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
			//clients[user_id] = 1;
			return user_id;
		}
        return -1;
}

//if successful register return user_id else -1
int validate_register(char* payload)
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
//create file + write filename in files.txt
int createFile(char* filename)
{
    char* fn;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    
    //open file
    FILE * fp = fopen("files.txt", "ab+");
    if (fp == NULL){
        perror("files open");
        return -1;
    }
    
    int file_id = 0;
    int val = 0;
    //read files
    while ((read = getline(&line, &len, fp)) != -1) {
        file_id = atoi(strtok (line,":"));
        fn = strtok(NULL, ":");
        val = is_equivalent_strings(fn, filename, "", "");
        if(val) break;
    }
    //if exist same filename
    if(val) {
        fclose(fp);
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

    //write to files
    if(file_id == 0){
        fprintf(fp, "%d:%s", ++file_id, filename);
    }else{
        fprintf(fp, "\n%d:%s", ++file_id, filename);
    }
    //close
    fclose(fp);
    return file_id;
}


int deleteFile(int file_id)
{
    int ret = remove(getFileName(file_id));
    if(ret != 0)
    {
        return -1;
    }

    //TODO

}

//return filename
char* getFileName(int file_id)
{
    char* fn;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    
    //open file
    FILE * fp = fopen("files.txt", "r");
    if (fp == NULL){
        perror("files open");
    }
    //read and search filename
    int id = 0;    
    while ((read = getline(&line, &len, fp)) != -1) {
        id = atoi(strtok (line,":"));
        fn = strtok(NULL, ":");
        if(id == file_id){
            fclose(fp);
            return terminateNull(fn);
        }
    }
    //close and return
    fclose(fp);
    return NULL;
}

//add user data to file_id
void addFileToUser(int file_id, int user_id)
{
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