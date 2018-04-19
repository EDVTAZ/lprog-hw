#include <stdlib.h>
#include <rope.h>
#include <ui.h>
#include <buffer.h>
//#include "../include/buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <parson.h>

BRES buffer_scroll(buffer* b, CMOVE_DIR dir){
    // TODO adjust what cursors are on screen
    if(dir == UP){
        if(b->bottom->prev && b->top->prev){
            b->bottom = b->bottom->prev;
            b->top = b->top->prev;
        } 
    }
    if(dir == DOWN){
        if(b->top->next && b->bottom->next){
            b->top = b->top->next;
            b->bottom = b->bottom->next;
        } 
    }
}

// should be used to trim the visible area, when l line was inserted
BRES buffer_trim(buffer* b, line* l){
    // TODO 
     if(b->bottom->prev) b->bottom = b->bottom->prev;
    return UPDATE;
}

// after deleting a line, there is space for one more
BRES buffer_extend(buffer* b, line* l){
    // TODO
    if(b->bottom->next) b->bottom = b->bottom->next;
    else if(b->top->prev) b->top = b->top->prev;

    return UPDATE;
}

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
    if(c) free(c);
}

// move cursor
CMOVE_RES cursor_move(cursor* c, CMOVE_DIR dir){
    switch(dir){
        case UP:
            if(c->own_line->prev && c->own_line != c->buf->first){
                if(c->own_line->prev) c->own_line = c->own_line->prev;
                if(rope_char_count(c->own_line->str) < c->pos) c->pos = rope_char_count(c->own_line->str);
                return aUP;
            }
            return aNONE;
            break;

        case DOWN:
            if(c->own_line->next && c->own_line != c->buf->last){
                if(c->own_line->next) c->own_line = c->own_line->next;
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
                if(c->own_line->prev && c->own_line != c->buf->first){
                    if(c->own_line->prev) c->own_line = c->own_line->prev;
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
                if(c->own_line->next && c->own_line != c->buf->last){
                    if(c->own_line->next) c->own_line = c->own_line->next;
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

    if(rope_char_count(c->own_line->str) >= c->buf->width-1) return FAILED;
//    if( rope_char_count(c->own_line->str) && rope_char_count(c->own_line->str) % c->buf->width == 0 ){
//        buffer_scroll(c->buf, DOWN);
//        buffer_trim(c->buf, c->own_line);
//    }

    rope_insert(c->own_line->str, c->pos, hstr);
    c->pos++;

    return UPDATE;
}

// insert line at cursor position with id
//CMOVE_RES cursor_insert_line(cursor* c, int id)

// delete character at cursor location (deleting endline not handled here)
CMOVE_RES cursor_del(cursor* c){
    if(c->pos == 0) return -1;

//    if( rope_char_count(c->own_line->str) != 1 && rope_char_count(c->own_line->str) % c->buf->width == 1 ){
//        buffer_scroll(c->buf, UP);
//        buffer_extend(c->buf, c->own_line);
//    }

    rope_del(c->own_line->str, c->pos-1, 1);
    c->pos--;
    return 0;
}


// BUFFER ---------------------------------------------------------------

// find line by id
line* bline_find(buffer* b, int id)
{
    line* lit = b->last;
    while(lit)
    {
        if(id == lit->id) return lit;

        if(lit != b->first) lit = lit->prev;
        else break;
    }

    return NULL;
}

// find cursor by id
cursor* bcursor_find(buffer* b, int id){
    if(b->own_curs->id == id) return b->own_curs;
    else for(int i=0; i<MAX_CURSOR_NUM; i++){
        if(b->peer_curss[i]->id == id) return b->peer_curss[i];
    }

    return NULL;
}

// initialize empty buffer with h height and w width
buffer* buffer_new(int id, int ver, int h, int w, int ui){
    buffer* b = malloc(sizeof(buffer));

    b->id = id;
    b->ver = ver;
    b->line_id_cnt = 0;

    b->height = h;
    b->width = w;

    b->num_lines = h;
    b->first = b->top = b->last = line_new(b->line_id_cnt++, NULL, NULL);
    line* pp = b->first;
    for(int i=1; i < b->height; i++){
        pp = b->bottom = line_new(b->line_id_cnt++, pp, NULL);
    }

    //b->own_curs = cursor_new(0, b, b->first, 0, 1);
    bcursor_new(b, 0, 0, 0);
    for(int i=0; i<MAX_CURSOR_NUM; i++){
        b->peer_curss[i] = NULL;
    }

    if(ui) b->u = ui_init(b);
    else b->u = NULL;

}

// initialize buffer from file with h height and w width
buffer* buffer_from_file(char* fname, int id, int h, int w, int ui){

    int fd = open(fname, O_RDONLY);
    if(fd < 0)
    {
        perror("Error opening file");
        return NULL;
    }

    buffer* b = malloc(sizeof(buffer));

    b->id = id;
    b->ver = 0;
    b->line_id_cnt = 0;

    b->height = h;
    b->width = w;

    b->num_lines = h;
    b->first = b->top = b->last = line_new(b->line_id_cnt++, NULL, NULL);
    line* pp = b->first;
    for(int i=1; i < b->height; i++){
        pp = b->bottom = line_new(b->line_id_cnt++, pp, NULL);
    }

    //b->own_curs = cursor_new(0, b, b->first, 0, 1);
    bcursor_new(b, 0, 0, 0);
    for(int i=0; i<MAX_CURSOR_NUM; i++){
        b->peer_curss[i] = NULL;
    }
    
    int c;
    int ret, upcnt=0;
    while(ret = read(fd, &c, 1))
    {
        if(ret == -1)
            perror("Error reading file");

        if((char)c == '\n'){
            bcursor_insert_line(b, 0);   
            upcnt++;
        }
        else bcursor_insert(b, 0, c);
    }
    while(upcnt--)
        bcursor_move(b, 0, UP);
    bcursor_find(b, 0)->pos=0;

    if(close(fd) == -1)
        perror("Error closing file");

    if(ui) b->u = ui_init(b);
    else b->u = NULL;

    return b;
}

// save contents of buffer to file
int buffer_save(char* fname, buffer* b){

    int fd;
    if((fd = open(fname, O_RDWR | O_TRUNC | O_CREAT, 00600)) < 0)
    {
        perror("Error opening file for writing");
        return -1;
    }

    line* lit = b->first;
    char buf[b->width+1];
    while(lit)
    {
        int len = rope_write_cstr(lit->str, buf);
        write(fd, buf, len-1);
        write(fd, "\n", 1);

        if(lit != b->last)
        {
            lit = lit->next;
        }
        else break;

    }

    close(fd);

    return 0;

}

// deletes buffer
void buffer_free(buffer* b){

    ui_free(b->u);

    cursor_free(b->own_curs);
    for(int i=0; i<MAX_CURSOR_NUM; i++)
    {
        cursor_free(b->peer_curss[i]);
    }

    line* lit = b->first;
    line* litnext;
    while(lit)
    {
        litnext = lit->next;
        line_free(lit);
        lit = litnext;
    }

    free(b);

}

// probly don't need this
//// insert line containing cstr after line with id, with the id new_id
//// cstr shall not contain newline characters and be null terminated
//// if new_id is -1 a new id will be automatically generated
//BRES buffer_insertl(buffer* b, char* cstr, int prev_id, int new_id){
//    // TODO
//    return UPDATE;
//}

// delete line
BRES buffer_deletel(buffer* b, line* l){
    // TODO handle other cursors

    buffer_extend(b, l);
    line_free(l);

    return UPDATE;
}

// create cursor with id in lid line at pos position
BRES bcursor_new(buffer* b, int id, int lid, int pos){

    // find line
    line* l = bline_find(b, lid);
    // TODO calculate on screen
    cursor* c = cursor_new(id, b, l, pos, 1);

    if(id == 0)
    {
        b->own_curs = c;
        return UPDATE;
    }
    //insert into peer curss
    for(int i=0; i<MAX_CURSOR_NUM; i++){
        if(!b->peer_curss[i])
        {
            b->peer_curss[i] = c;
            break;
        }
    }

    return UPDATE;
}

// move cursor
// check if we have to scroll or show a new cursor on screen
BRES bcursor_move(buffer* b, int id, CMOVE_DIR dir){

    cursor* c = bcursor_find(b, id);
    CMOVE_RES res = cursor_move(c, dir);
    if(res == aUP && b->top == c->own_line && c == b->own_curs) buffer_scroll(b, UP);
    if(res == aDOWN && b->bottom== c->own_line && c == b->own_curs) buffer_scroll(b, DOWN);

    if(b->u) ui_update(b->u);
    // TODO
    return UPDATE;
}

// isnert character at cursor location
BRES bcursor_insert(buffer* b, int id, char chr){
    
    cursor* c = bcursor_find(b, id);
    cursor_insert(c, chr);

    if(b->u) ui_update(b->u);
    // TODO correct positions of cursors with greater position than ours
    return UPDATE;
}


// isnert new line at cursor location
// if cursor is at pos 0, it will stay in the line and the new line will be above the current line
// if the cursor is somewhere else, it will be moved to pos 0 in the new line, and the contents to the right of the cursor will be moved into the new line
BRES bcursor_insert_line(buffer* b, int id){
    
    b->num_lines++;
    cursor* c = bcursor_find(b, id);

    if(c->pos == 0){
        line* ll = line_new(b->line_id_cnt++, c->own_line->prev, c->own_line);
        if(c->own_line == b->first) b->first = ll;

        if(c->own_line == b->top){
            buffer_scroll(b, UP);
        }
        else {
            buffer_trim(b, ll);
        }
        if(c->own_line == b->first) b->first = c->own_line;
        
        if(b->u) ui_update(b->u);
        return UPDATE;
    }

    line* ll = line_new(b->line_id_cnt++, c->own_line, c->own_line->next);
    if(c->own_line == b->last) b->last = ll;

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

    //c->own_line = c->own_line->next;
    bcursor_move(b, id, DOWN);

    if(b->u) ui_update(b->u);
    return UPDATE;
}

// delete character at cursor location
BRES bcursor_del(buffer* b, int id){
    
    cursor* c = bcursor_find(b, id);
    if(c->pos > 0) cursor_del(c);
    else{
        if(c->own_line == b->first) return FAILED;
        b->num_lines--;
        bcursor_move(b, id, LEFT);
        if(c->own_line->next == b->last) b->last = c->own_line;
        if( rope_write_cstr(c->own_line->next->str, b->sp)>1 ){
            rope_insert(c->own_line->str, rope_char_count(c->own_line->str), b->sp);
        }
        buffer_deletel(b, c->own_line->next);
    }

    if(b->u) ui_update(b->u);
    // TODO correct positions of cursors with greater position than ours
    return UPDATE;
}

buffer* buffer_deserialize(char* serd, int u){
    
    JSON_Value *root_value = json_parse_string(serd);
    JSON_Object *root_object = json_value_get_object(root_value);

    buffer* b = malloc(sizeof(buffer));

    //// buffer params
    int first, last, top, bottom;

    b->id = (int)json_object_get_number(root_object, "id");
    b->ver = (int)json_object_get_number(root_object, "ver");
    b->num_lines = (int)json_object_get_number(root_object, "num_lines");
    b->line_id_cnt = (int)json_object_get_number(root_object, "line_id_cnt");
    b->height = (int)json_object_get_number(root_object, "height");
    b->width = (int)json_object_get_number(root_object, "width");
    first = (int)json_object_get_number(root_object, "first");
    last = (int)json_object_get_number(root_object, "last");
    top = (int)json_object_get_number(root_object, "top");
    bottom = (int)json_object_get_number(root_object, "bottom");

    //// lines

    // get first line
    b->first = line_new(first, NULL, NULL);

    // set string value of first line
    snprintf(b->sp, SP_SIZE, "lines.%d.str", first);
    rope_insert(b->first->str, 0, json_object_dotget_string(root_object, b->sp));

    // create next line and set string value
    snprintf(b->sp, SP_SIZE, "lines.%d.next", first);
    int n = (int)json_object_dotget_number(root_object, b->sp);
    line* lit = line_new(n, b->first, NULL);
    snprintf(b->sp, SP_SIZE, "lines.%d.str", lit->id);
    rope_insert(lit->str, 0, json_object_dotget_string(root_object, b->sp));

    // get next line path to scratchpad
    snprintf(b->sp, SP_SIZE, "lines.%d.next", lit->id);
    while( json_object_dothas_value(root_object, b->sp) )
    {
        // create next line
        n = json_object_dotget_number(root_object, b->sp);
        lit = line_new(n, lit, NULL);

        // set string value of next line
        snprintf(b->sp, SP_SIZE, "lines.%d.str", lit->id);
        rope_insert(lit->str, 0, json_object_dotget_string(root_object, b->sp));

        // get next line path to scratchpad
        snprintf(b->sp, SP_SIZE, "lines.%d.next", lit->id);
    }
    // set top/bottom, first/last in buffer ORDER IS IMPORTANT
    b->last = lit;
    b->bottom = bline_find(b, bottom);
    b->last = bline_find(b, last);
    b->top = bline_find(b, top);

    //// cursors
    // own cursor
    int id = (int)json_object_dotget_number(root_object, "own_cursor.id");
    int lid = (int)json_object_dotget_number(root_object, "own_cursor.own_line");
    int pos = (int)json_object_dotget_number(root_object, "own_cursor.pos");
    bcursor_new(b, id, lid, pos);
    // peer cursors
    for(int i=0; i<MAX_CURSOR_NUM; i++)
        b->peer_curss[i] = NULL;
    for(int i=0; i<MAX_CURSOR_NUM; i++)
    {
        snprintf(b->sp, SP_SIZE, "peer_curss.%d", i);
        if( json_object_dothas_value(root_object, b->sp) )
        {
            id = i;
            snprintf(b->sp, SP_SIZE, "peer_curss.%d.own_line", i);
            lid = (int)json_object_dotget_number(root_object, b->sp);
            snprintf(b->sp, SP_SIZE, "peer_curss.%d.pos", i);
            pos = (int)json_object_dotget_number(root_object, b->sp);
            bcursor_new(b, id, lid, pos);
        }
    }

    if(u)
    {
        b->u = ui_init(b);
        ui_update(b->u);
    }
    else b->u = NULL;

    return b;
}

char* buffer_serialize(buffer* b){


    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;
    //// buffer attrs
    // buffer id
    json_object_set_number(root_object, "id", b->id);
    // buffer version counter
    json_object_set_number(root_object, "ver", b->ver);
    // buffer number of lines
    json_object_set_number(root_object, "num_lines", b->num_lines);
    // buffer line_id_cnt
    json_object_set_number(root_object, "line_id_cnt", b->line_id_cnt);
    // buffer height
    json_object_set_number(root_object, "height", b->height);
    // buffer width
    json_object_set_number(root_object, "width", b->width);
    // buffer first line id
    json_object_set_number(root_object, "first", b->first->id);
    // buffer last line id
    json_object_set_number(root_object, "last", b->last->id);
    // buffer top line id
    json_object_set_number(root_object, "top", b->top->id);
    // buffer bottom line id
    json_object_set_number(root_object, "bottom", b->bottom->id);
    //// lines
    //JSON_Value *lines_jval = NULL, *val_parent;
    //JSON_Object *obj = NULL;
    line* lit = b->first;
    char path[2048];
    while(lit){
        int slen = rope_write_cstr(lit->str, b->sp);

        if(lit->next)
        {
            snprintf(path, 2048, "lines.%d.next", lit->id);
            json_object_dotset_number(root_object, path, lit->next->id);
        }
        if(lit->prev)
        {
            snprintf(path, 2048, "lines.%d.prev", lit->id);
            json_object_dotset_number(root_object, path, lit->prev->id);
        }
        snprintf(path, 2048, "lines.%d.str", lit->id);
        json_object_dotset_string(root_object, path, b->sp);

        lit = lit->next;
    }
    //// cursors
    cursor* c = b->own_curs;
    // id
    json_object_dotset_number(root_object, "own_cursor.id", c->id);
    // own_line
    json_object_dotset_number(root_object, "own_cursor.own_line", c->own_line->id);
    // position
    json_object_dotset_number(root_object, "own_cursor.pos", c->pos);
    // on_screen
    json_object_dotset_number(root_object, "own_cursor.on_screen", c->on_screen);

    for(int i=0; i<MAX_CURSOR_NUM; i++){
        if(c = b->peer_curss[i]){
            // own_line
            snprintf(path, 2048, "peer_curss.%d.own_line", c->id);
            json_object_dotset_number(root_object, path, c->own_line->id);
            // position
            snprintf(path, 2048, "peer_curss.%d.pos", c->id);
            json_object_dotset_number(root_object, path, c->pos);
            // on_screen
            snprintf(path, 2048, "peer_curss.%d.on_screen", c->id);
            json_object_dotset_number(root_object, path, c->on_screen);
        }
    }

    serialized_string = json_serialize_to_string(root_value);
    //puts(serialized_string);
    //printf("%d", sizeof(serialized_string));
    //json_free_serialized_string(serialized_string);
    json_value_free(root_value);
    return serialized_string;
}
