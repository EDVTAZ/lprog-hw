#include <data.h>
#include <parson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


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
        //free json
        json_value_free(root_value);
        return -2;
    }
    
    //check if username and password is correct
    int user_id = 1;
    int val = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        username = strtok (line,":");
        password = strtok (NULL, ":");
        val = (int) is_equivalent_strings(name, username, pass, terminateNull(password));
        if(val) break;
        user_id++;
    }

    //close file
    fclose(fp);
    //free json
    json_value_free(root_value);
    //return when user's data is correct
    if(val) {
        //clients[user_id] = 1;
        return user_id;
    }
    //return when user's data is incorrect
    return -1;
}

//if successful register return user_id else -1
int validate_register(char* payload)
{ 
    JSON_Value *root_value = json_parse_string(payload);
    JSON_Object *root_object = json_value_get_object(root_value);

    const char *name = json_object_get_string(root_object, "name");
    const char *pass = json_object_get_string(root_object, "pass");
    //free json
    json_value_free(root_value);
    
    char* username;
    char* password;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    
    //open file
    FILE * fp = fopen("userdata.txt", "ab+");
    if (fp == NULL){
        perror("userdata open");
        return -2;
    }
    
    //check username exist
    int user_id = 1;
    int val = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        username = strtok (line,":");
         val = is_equivalent_strings(name, username, "", "");
        if(val) break;
        user_id++;
    }
    //return when username already exist
    if(val) {
        fclose(fp);
        return -1;
        
    }
    //write user's data to file
    if(user_id == 1){
         fprintf(fp, "%s:%s", name, pass);
    }else{
        fprintf(fp, "\n%s:%s", name, pass);
    }
    //close file
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
        return -2;
    }
    //check filename exist
    int file_id = 0;
    int val = 0;  
    while ((read = getline(&line, &len, fp)) != -1) {
        file_id = atoi(strtok (line,":"));
        fn = strtok(NULL, ":");
        val = is_equivalent_strings(fn, filename, "", "");
        if(val) break;
    }
    //return when filename already exist
    if(val) {
        fclose(fp);
        return -1;
        
    }
    //write data to file
    if(file_id == 0){
        fprintf(fp, "%d:%s", ++file_id, filename);
    }else{
        fprintf(fp, "\n%d:%s", ++file_id, filename);
    }
    //create and close file
    int fd = creat(filename, (mode_t)0664);
    if (fd == -1) {
        perror("file creating");
        return -3;
    }
    close(fd);
    //close file
    fclose(fp);
    return file_id;
}


int deleteFile(int file_id)
{
    char * line_copy;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    
    //get filename to file_id
    char* filename = getFileName(file_id);

    //rename the file
    rename("files.txt", "files_temp.txt");

    //open file
    FILE * fp_temp = fopen("files_temp.txt", "r+");
    if (fp_temp == NULL){
        perror("files_temp.txt open");
        return -3;
    }

    //create file
    FILE * fp = fopen("files.txt", "w");
    if (fp == NULL){
        perror("files.txt open");
        return -2;
    }
    

    //count lines from files_tmp.txt
    int lines_number = 0;
    while ((read = getline(&line, &len, fp_temp)) != -1) {
        lines_number++;
    }

    //go to beginning of file
    fseek(fp_temp, 0, SEEK_SET);

    //read data from files_temp.txt and write to files.txt
    int id = 0;
    int count = 0;
    while ((read = getline(&line, &len, fp_temp)) != -1) {
        count++;
        line_copy = strdup(line);
        id = atoi(strtok (line_copy,":"));
        free(line_copy);
        if(id != file_id){
            if(count != lines_number){
                fprintf(fp, "%s", line);
            }else{
                fprintf(fp, "%s", terminateNull(line));
            }
        }
    }
    //close files
    fclose(fp_temp);
    fclose(fp);
    //delete temp and user file
    int err = 0;
    if(remove("files_temp.txt") != 0){
        perror("delete userdata_temp.txt");
        err = -1;
    }
    if(remove(filename) != 0){
        perror("delete user's file");
        err = -1;
    }
    return err;
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
    //search filename to file_id
    int id = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        id = atoi(strtok (line,":"));
        fn = strtok(NULL, ":");
        if(id == file_id){
            fclose(fp);
            return terminateNull(fn);
        }
    }
    //close file
    fclose(fp);
    return NULL;
}

//TODO: check if user already has access
//add user data to file_id
int addFileToUser(int file_id, int user_id)
{
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    
    //rename the file
    rename("userdata.txt", "userdata_temp.txt");

    //open files
    FILE * fp_temp = fopen("userdata_temp.txt", "r+");
    if (fp_temp == NULL){
        perror("userdata_temp.txt open");
        return -3;
    }

    //create file
    FILE * fp = fopen("userdata.txt", "w");
    if (fp == NULL){
        perror("userdatatxt open");
        return -2;
    }
    
    //read userdata from userdata_temp.txt and write to userdata.txt
    int i = 1;
    while ((read = getline(&line, &len, fp_temp)) != -1) {
        if(i == user_id) {
            fprintf(fp, "%s", line);
            if(strstr(line, "\n") != NULL){
                fseek(fp, -1, SEEK_CUR);
                fprintf(fp, ":%d\n", file_id);
            }else{
                fprintf(fp, ":%d", file_id);
            }
        }else{
            fprintf(fp, "%s", line);
        }
        i++;
    }
    //close files
    fclose(fp_temp);
    fclose(fp);
    //delete temp file
    int err = 0;
    if(remove("userdata_temp.txt") != 0){
        perror("delete userdata_temp.txt");
        err = -1;
    }
    return err;
}

//TODO: get user's files
