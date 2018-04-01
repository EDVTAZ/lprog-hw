// AUTHOR: Szekely Gabor

#ifndef buffer_h
#define buffer_h

#include <rope.h>


// LINE ---------------------------------------------------------------

typedef struct{
    
    rope* str;
    int id;

    line* prev;
    line* next;
} line;

// create new line with id and insert it into the chain between prev and next
line* line_new(int id, line* prev, line* next);

// delete line, connecting the chain (prev -- next), returns prev
line* line_free(line* l);


// CURSOR ---------------------------------------------------------------

typedef struct{
    
    line* own_line;
    int pos;
    int id;

} cursor;

// create new cursor wit id, in the line with lid at pos position
cursor* cursor_new(int id, int lid, int pos);

// delete cursor
void cursor_free(cursor* c);

// cursor movement results
// UP: cursor moved up one line (might have moved horizontally too)
// DOWN: same as up but the other direction
// HORIZONTAL: cursor moved, but is still in the same line
// NONE: cursor didn't move
typedef enum {UP, DOWN, HORIZONTAL, NONE} CMOVE_RES;
// direction to move cursor
typedef enum {UP, DOWN, LEFT, RIGHT} CMOVE_DIR;

// move cursor
CMOVE_RES cursor_move(cursor* c, CMOVE_DIR dir);

// isnert character at cursor location
int cursor_insert(cursor* c, char c);

// delete character at cursor location
int cursor_del(cursor* c);


// BUFFER ---------------------------------------------------------------
// it is the buffers responsibility to keep the network and UI modules updated (through their respective interfaces) of all updates made to the contents and the cursors' positions
// in short: the buffer pushes changes

typedef struct{
    
    size_t num_lines;
    int line_id_cnt;

    // very first and last line of the buffer
    line* first;
    line* last;

    // top and bottom visible lines
    line* top;
    line* bottom;

    // cursors
    cursor* own_curs;
    cursor[MAX_CURSOR_NUM] peer_curss;

} buffer;

// initialize empty buffer
buffer* buffer_new();

// initialize buffer from file
buffer* buffer_from_file(FILE* fp);

// deletes buffer
void buffer_free(buffer* b);

// insert line containing cstr after line with id, with the id new_id
// cstr shall not contain newline characters and be null terminated
// if new_id is -1 a new id will be automatically generated
// returns the new id on success, -1 on failure
int buffer_insertl(buffer* b, char* cstr, int prev_id, int new_id);

// create cursor with id in lid line at pos position
// return -1 on failure
int bcursor_new(int id, int lid, int pos);

// move cursor
CMOVE_RES bcursor_move(int id, CMOVE_DIR dir);

// isnert character at cursor location
int bcursor_insert(int id, char c);

// delete character at cursor location
int bcursor_del(int id);



#endif
