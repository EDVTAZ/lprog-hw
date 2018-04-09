#include <stdlib.h>
#include <rope.h>
#include <ui.h>
#include <buffer.h>
//#include "../include/buffer.h"


// LINE ---------------------------------------------------------------

// create new line with id and insert it into the chain between prev and next
line* line_new(int id, line* prev, line* next){
    line* l = malloc(sizeof(line));

    l->id = id;

    l->str = rope_new();

    l->prev = prev;
    if(prev) l->prev->next = l;
    l->next = next;
    if(next) l->next->prev = l;

    return l;
}

// delete line, connecting the chain (prev -- next), returns prev
line* line_free(line* l){
    rope_free(l->str);

    if(l->prev) l->prev->next = l->next;
    if(l->next) l->next->prev = l->prev;

    line* p = l->prev;
    free(l);
    return p;
}


// CURSOR ---------------------------------------------------------------

char hstr[2];

// create new cursor wit id, in the line l at pos position
cursor* cursor_new(int id, buffer* buf, line* l, int pos, int os){
    cursor* c = malloc(sizeof(cursor));

    c->id = id;
    c->buf = buf;
    c->own_line = l;
    c->pos = pos;

    c->on_screen = os;
}

// delete cursor
void cursor_free(cursor* c){
    free(c);
}

// move cursor
CMOVE_RES cursor_move(cursor* c, CMOVE_DIR dir){
    switch(dir){
        case UP:
            if(c->own_line->prev){
                c->own_line = c->own_line->prev;
                if(rope_char_count(c->own_line->str) < c->pos) c->pos = rope_char_count(c->own_line->str);
                return aUP;
            }
            return aNONE;
            break;

        case DOWN:
            if(c->own_line->next){
                c->own_line = c->own_line->next;
                if(rope_char_count(c->own_line->str) < c->pos) c->pos = rope_char_count(c->own_line->str);
                return aDOWN;
            }
            return aNONE;
            break;

        case LEFT:
            // cursor stays in the same line
            if(c->pos > 0){
                c->pos--;
                return aHORIZONTAL;
            }
            // cursor moves into the previous line
            else{
                if(c->own_line->prev){
                    c->own_line = c->own_line->prev;
                    c->pos = rope_char_count(c->own_line->str);
                    return aUP;
                }
                return aNONE;
            }
            break;

        case RIGHT:
            // cursor stays in the same line (no -1 after rope char count, this allows us to put the cursor on top of the newline char, that is not part of the rope)
            if(c->pos < rope_char_count(c->own_line->str)){
                c->pos++;
                return aHORIZONTAL;
            }
            // cursor moves into the previous line
            else{
                if(c->own_line->next){
                    c->own_line = c->own_line->next;
                    c->pos = 0;
                    return aDOWN;
                }
                return aNONE;
            }
            break;
    }
}

// insert character at cursor location
CMOVE_RES cursor_insert(cursor* c, char chr){
    hstr[0] = chr;
    rope_insert(c->own_line->str, c->pos, hstr);
    c->pos++;

    return UPDATE;
}

// insert line at cursor position with id
//CMOVE_RES cursor_insert_line(cursor* c, int id)

// delete character at cursor location (deleting endline not handled here)
CMOVE_RES cursor_del(cursor* c){
    if(c->pos == 0) return -1;

    rope_del(c->own_line->str, c->pos-1, 1);
    c->pos--;
    return 0;
}


// BUFFER ---------------------------------------------------------------

// initialize empty buffer with h height and w width
buffer* buffer_new(int id, int cid, int ver, int h, int w){
    buffer* b = malloc(sizeof(buffer));

    b->id = id;
    b->ver = ver;
    b->line_id_cnt = 0;

    b->height = h;
    b->width = w;

    b->num_lines = 1;
    b->first = b->top = b->last = line_new(b->line_id_cnt++, NULL, NULL);
    line* pp = b->first;
    for(int i=1; i < b->height; i++){
        pp = b->bottom = line_new(b->line_id_cnt++, pp, NULL);
    }

    b->own_curs = cursor_new(cid, b, b->first, 0, 1);
    for(int i=0; i<MAX_CURSOR_NUM; i++){
        b->peer_curss[i] = NULL;
    }

    b->u = ui_init(b);

}

// initialize buffer from file with h height and w width
buffer* buffer_from_file(FILE* fp, int h, int w){
    // TODO
}

// save contents of buffer to file
int buffer_save(FILE* fp){
    // TODO
    return -1;
}

// deletes buffer
void buffer_free(buffer* b){
    // TODO
}

// find cursor by id
cursor* bcursor_find(buffer* b, int id){
    if(b->own_curs->id == id) return b->own_curs;
    else for(int i=0; i<MAX_CURSOR_NUM; i++){
        if(b->peer_curss[i]->id == id) return b->peer_curss[i];
    }

    return NULL;
}


// insert line containing cstr after line with id, with the id new_id
// cstr shall not contain newline characters and be null terminated
// if new_id is -1 a new id will be automatically generated
BRES buffer_insertl(buffer* b, char* cstr, int prev_id, int new_id){
    // TODO
    return UPDATE;
}

// delete line
BRES buffer_deletel(buffer* b, int lid){
    // TODO
    return UPDATE;
}

// create cursor with id in lid line at pos position
BRES bcursor_new(buffer* b, int id, int lid, int pos){
    // TODO
    return UPDATE;
}

// move cursor
// check if we have to scroll or show a new cursor on screen
BRES bcursor_move(buffer* b, int id, CMOVE_DIR dir){

    cursor* c = bcursor_find(b, id);
    cursor_move(c, dir);

    ui_update(b->u);
    // TODO
    return UPDATE;
}

// isnert character at cursor location
BRES bcursor_insert(buffer* b, int id, char chr){
    
    cursor* c = bcursor_find(b, id);
    cursor_insert(c, chr);

    ui_update(b->u);
    // TODO correct positions of cursors with greater position than ours
    return UPDATE;
}

BRES buffer_scroll(buffer* b, CMOVE_DIR dir){
    // TODO adjust what cursors are on screen
    if(dir == UP){
        b->top = b->top->prev;
        b->bottom = b->bottom->prev;
    }
    if(dir == DOWN){
        b->top = b->top->next;
        b->bottom = b->bottom->next;
    }
}

// should be used to trim the visible area, when l line was inserted
BRES buffer_trim(buffer* b, line* l){
    // TODO 
    b->bottom = b->bottom->prev;
    return UPDATE;
}

// isnert new line at cursor location
// if cursor is at pos 0, it will stay in the line and the new line will be above the current line
// if the cursor is somewhere else, it will be moved to pos 0 in the new line, and the contents to the right of the cursor will be moved into the new line
BRES bcursor_insert_line(buffer* b, int id){
    
    cursor* c = bcursor_find(b, id);

    if(c->pos == 0){
        line* ll = line_new(b->line_id_cnt++, c->own_line->prev, c->own_line);

        if(c->own_line == b->top){
            buffer_scroll(b, UP);
        }
        else buffer_trim(b, ll);
        if(c->own_line == b->first) b->first = c->own_line;
        
        ui_update(b->u);
        return UPDATE;
    }

    line* ll = line_new(b->line_id_cnt++, c->own_line, c->own_line->next);

    if(c->pos != rope_char_count(c->own_line->str)){
        // TODO move other cursors with us
        rope_free(ll->str);
        ll->str = rope_copy(ll->prev->str);
        rope_del(ll->prev->str, c->pos, rope_char_count(ll->str)-c->pos);
        rope_del(ll->str, 0, c->pos);
        
    }

    c->pos = 0;

    if(c->own_line == b->bottom){
        buffer_scroll(b, DOWN);
    }
    else buffer_trim(b, ll);
    if(c->own_line == b->last) b->last = c->own_line;

    c->own_line = c->own_line->next;

    ui_update(b->u);
    return UPDATE;

    
}

// delete character at cursor location
BRES bcursor_del(buffer* b, int id){
    
    cursor* c = bcursor_find(b, id);
    cursor_del(c);

    ui_update(b->u);
    // TODO correct positions of cursors with greater position than ours
    return UPDATE;
}
