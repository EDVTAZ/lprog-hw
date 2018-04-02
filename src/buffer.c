#include <stdlib.h>
#include <rope.h>
#include <ui.h>
#include <buffer.h>


// LINE ---------------------------------------------------------------

// create new line with id and insert it into the chain between prev and next
line* line_new(int id, line* prev, line* next){
    line* l = malloc(sizeof(line));

    l->id = id;

    l->str = rope_new();

    l->prev = prev;
    l->prev->next = l;
    l->next = next;
    l->next->prev = l;

    return l;
}

// delete line, connecting the chain (prev -- next), returns prev
line* line_free(line* l){
    rope_free(l->str);

    l->prev->next = l->next;
    l->next->prev = l->prev;

    line* p = l->prev;
    free(l);
    return p;
}


// CURSOR ---------------------------------------------------------------

char hstr[2];

// create new cursor wit id, in the line l at pos position
cursor* cursor_new(int id, buffer* buf; line* l, int pos){
    cursor* c = malloc(sizeof(cursor));

    c->id = id;
    c->buf = buf;
    c->own_line = l;
    c->pos = pos;
}

// delete cursor
void cursor_free(cursor* c){
    free(c);
}

// move cursor
CMOVE_RES cursor_move(cursor* c, CMOVE_DIR dir){
    switch(dir){
        case CMOVE_DIR.UP:
            if(c->own_line->prev){
                c->own_line = c->own_line->prev;
                return CMOVE_RES.UP;
            }
            return CMOVE_RES.NONE;
            break;

        case CMOVE_DIR.DOWN:
            if(c->own_line->next){
                c->own_line = c->own_line->next;
                return CMOVE_RES.DOWN;
            }
            return CMOVE_RES.NONE;
            break;

        case CMOVE_DIR.LEFT:
            // cursor stays in the same line
            if(c->pos > 0){
                c->pos--;
                return CMOVE_RES.HORIZONTAL;
            }
            // cursor moves into the previous line
            else{
                if(c->own_line->prev){
                    c->own_line = c->own_line->prev;
                    c->pos = rope_char_count(c->own_line->str);
                    return CMOVE_RES.UP;
                }
                return CMOVE_RES.NONE;
            }
            break;

        case CMOVE_DIR.RIGHT:
            // cursor stays in the same line (no -1 after rope char count, this allows us to put the cursor on top of the newline char, that is not part of the rope)
            if(c->pos < rope_char_count(c->own_line->str)){
                c->pos++;
                return CMOVE_RES.HORIZONTAL;
            }
            // cursor moves into the previous line
            else{
                if(c->own_line->next){
                    c->own_line = c->own_line->next;
                    c->pos = 0;
                    return CMOVE_RES.DOWN;
                }
                return CMOVE_RES.NONE;
            }
            break;
    }
}

// insert character at cursor location
CMOVE_RES cursor_insert(cursor* c, char chr){
    hstr[0] = chr;
    rope_insert(c->own_line->str, c->pos, hstr);
    c->pos++;

    return CMOVE_RES.HORIZONTAL;
}

// insert line at cursor position with id
CMOVE_RES cursor_insert_line(cursor* c, int id)

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
BRES bcursor_new(buffer* b, int id, int lid, int pos);

// move cursor
BRES bcursor_move(buffer* b, int id, CMOVE_DIR dir);

// isnert character at cursor location
BRES bcursor_insert(buffer* b, int id, char c);

// isnert new line at cursor location
// if cursor is at pos 0, it will stay in the line and the new line will be above the current line
// if the cursor is somewhere else, it will be moved to pos 0 in the new line, and the contents to the right of the cursor will be moved into the new line
BRES bcursor_insert_line(buffer* b, int id);

// delete character at cursor location
BRES bcursor_del(buffer* b, int id);
