#ifndef buffer_h
#define buffer_h

#include <rope.h>
#include <ui.h>

typedef struct line line;
typedef struct cursor cursor;
typedef struct buffer buffer;

// this might not be a very good idea...
#ifndef MAX_CURSOR_NUM
#define MAX_CURSOR_NUM 20
#endif

#define SP_SIZE 2048

// LINE ---------------------------------------------------------------
typedef enum crp {ABOVE, BELLOW, SAME} CRP;

// result ofbuffer operation
// FAILED: operation failed
// UPDATE: operation successful, UI update needed
// SUCCESS: operation successful, no UI updgrade needed
typedef enum bres{FAILED, UPDATE, SUCCESS} BRES;


struct line{
    
    int id;
    // rope structure containing the line
    rope* str;

    // neighbouring lines
    struct line* prev;
    struct line* next;

    // on screen info
    int on_screen;
    CRP where;
    
};

// create new line with id and insert it into the chain between prev and next
line* line_new(int id, line* prev, line* next);

// delete line, connecting the chain (prev -- next), returns prev
line* line_free(line* l);


// CURSOR ---------------------------------------------------------------

struct cursor{
    
    int id;
    // buffer of cursor
    buffer* buf;
    // the line the cursor is currently on
    line* own_line;
    // the x index of the cursor
    int pos;

};

// create new cursor wit id, in the line with lid at pos position
cursor* cursor_new(int id, buffer* buf, line* l, int pos);

// delete cursor
void cursor_free(cursor* c);

// cursor movement results
// UP: cursor moved up one line (might have moved horizontally too)
// DOWN: same as up but the other direction
// HORIZONTAL: cursor moved, but is still in the same line
// NONE: cursor didn't move
typedef enum cmove_res{aUP, aDOWN, aHORIZONTAL, aNL_UP, aNL_DOWN, aNONE} CMOVE_RES;
// direction to move cursor
typedef enum cmove_dir{UP, DOWN, LEFT, RIGHT} CMOVE_DIR;

// move cursor
CMOVE_RES cursor_move(cursor* c, CMOVE_DIR dir);

// isnert character at cursor location
BRES cursor_insert(cursor* c, char chr);

// delete character at cursor location
BRES cursor_del(cursor* c);


// BUFFER ---------------------------------------------------------------

struct buffer{
    
    int id;
    // version of the file
    int ver;
    int num_lines;
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
    // own_curs is local, always has id=0
    cursor* own_curs;
    // this should be in syncrhon across all clients and the server
    cursor* peer_curss[MAX_CURSOR_NUM];

    // connected ui
    struct ui* u;
    // 1 if we are in server mode
    int server_mode;

    // scratch pad cstr
    char sp[SP_SIZE];

};

// initialize empty buffer with h height and w width
// if ui is 0, then there won't be a ui (for server side)
buffer* buffer_new(int id, int ver, int h, int w, int ui);

// initialize buffer from file with h height and w width
buffer* buffer_from_file(char* fname, int id, int h, int w, int ui);

// save contents of buffer to file
int buffer_save(char* fname, buffer* b);

// deletes buffer
void buffer_free(buffer* b);

// insert line containing cstr after line with id, with the id new_id
// cstr shall not contain newline characters and be null terminated
// if new_id is -1 a new id will be automatically generated
BRES buffer_insertl(buffer* b, char* cstr, int prev_id, int new_id);

// delete line
BRES buffer_deletel(buffer* b, line* l);

// create cursor with id in lid line at pos position
BRES bcursor_new(buffer* b, int id, int lid, int pos);

// copy own_curs into peer_curss with id
// // USAGE
// this is for server side
// when a new peer connects, the server does the following:
// serializes buffer and sends it to client (this way the own_curs of the server becomes the own_curs of the new peer)
// now the server needs to create a peer cursor that will correspond to the just connected peer, this is what this function does
// now the server just needs to send over the id, and line_id and position of the new peer cursor to the rest of the peers
BRES bcursor_copy_own(buffer* b, int id);

// delete cursor with id
BRES bcursor_free(buffer* b, int id);

// move cursor
BRES bcursor_move(buffer* b, int id, CMOVE_DIR dir);

// isnert character at cursor location
BRES bcursor_insert(buffer* b, int id, char chr);

// isnert new line at cursor location
// if cursor is at pos 0, it will stay in the line and the new line will be above the current line
// if the cursor is somewhere else, it will be moved to pos 0 in the new line, and the contents to the right of the cursor will be moved into the new line
BRES bcursor_insert_line(buffer* b, int id);

// delete character at cursor location
BRES bcursor_del(buffer* b, int id);

// add ui to buffer
void buffer_add_ui(buffer* b);

//// serialization for synchronization between server and client
// serialize
char* buffer_serialize(buffer* b);

// deserialize
buffer* buffer_deserialize(char* serd, int u);

#endif
