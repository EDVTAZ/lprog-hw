#ifndef data_h
#define data_h

//replace \n to 0
char* terminateNull(char* s);

//username and password is equivalent
int is_equivalent_strings(const char* a1, const char* a2, const char* b1, const char* b2);

//if valid user data return user_id else -1
int validate_user(char *payload);

//if successful register return user_id else -1
int validate_register(char* payload);

//if successful create return file id else -1
//create file + write filename in files.txt
int createFile(char* filename);

//delete file + delete filename from files.txt
int deleteFile(int file_id);

//return filename
char* getFileName(int file_id);

//add user data to file_id
void addFileToUser(int file_id, int user_id);

#endif