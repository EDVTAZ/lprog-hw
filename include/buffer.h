// AUTHOR: Szekely Gabor

#ifndef buffer_h
#define buffer_h

#include <rope.h>


// LINE ---------------------------------------------------------------

typedef struct{
    
    int id;
    // rope structure containing the line
    rope* str;

    // neighbouring lines
    line* prev;
    line* next;
} line;

// create new line with id and insert it into the chain between prev and next
line* line_new(int id, line* prev, line* next);

// delete line, connecting the chain (prev -- next), returns prev
line* line_free(line* l);


// CURSOR ---------------------------------------------------------------

typedef struct{
    
    int id;
    // the line the cursor is currently on
    line* own_line;
    // the x index of the cursor
    int pos;
    // 1 if the cursor is visible on the screen, 0 otherwise
    int on_screen;

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

typedef struct{
    
    int id;
    // version of the file
    int ver;
    size_t num_lines;
    int line_id_cnt;

    // dimensions of the ui
    int height;
    int width;

    // very first and last line of the buffer
    line* first;
    line* last;

    // top and bottom visible lines
    line* top;
    line* bottom;

    // cursors
    cursor* own_curs;
    cursor[MAX_CURSOR_NUM] peer_curss;

    // connected ui
    ui* u;

} buffer;

// initialize empty buffer with h height and w width
buffer* buffer_new(int h, int w);

// initialize buffer from file with h height and w width
buffer* buffer_from_file(FILE* fp, int h, int w);

// save contents of buffer to file
int buffer_save(FILE* fp);

// deletes buffer
void buffer_free(buffer* b);

// result of buffer operation
// FAILED: operation failed
// UPDATE: operation successful, UI update needed
// SUCCESS: operation successful, no UI updgrade needed
typedef enum{FAILED, UPDATE, SUCCESS} BRES;

// insert line containing cstr after line with id, with the id new_id
// cstr shall not contain newline characters and be null terminated
// if new_id is -1 a new id will be automatically generated
BRES buffer_insertl(buffer* b, char* cstr, int prev_id, int new_id);

// delete line
BRES buffer_deletel(buffer* b, int lid);

// create cursor with id in lid line at pos position
BRES bcursor_new(int id, int lid, int pos);

// move cursor
BRES bcursor_move(int id, CMOVE_DIR dir);

// isnert character at cursor location
BRES bcursor_insert(int id, char c);

// delete character at cursor location
BRES bcursor_del(int id);



#endif
