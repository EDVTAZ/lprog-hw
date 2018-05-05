#include <stdlib.h>
#include <rope.h>
#include <ui.h>
#include <buffer.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <parson.h>

void binit_onscreen_info(buffer* b)
{
    line *lit = b->first;
    
    if(b->first == b->top) lit->on_screen = 1;
    else lit->on_screen = 0;

    if(b->own_curs->own_line == lit) lit->where = SAME;
    else lit->where = ABOVE;

    int os = lit->on_screen;
    CRP rel = lit->where;
    while(lit = lit->next)
    {
        if(os)
        {
            lit->on_screen = os;
            if(lit == b->bottom) os = 0;
        }
        else
        {
            if(lit == b->top) os = 1;
            lit->on_screen = os;
        }

        if(rel == ABOVE && lit == b->own_curs->own_line)
        {
            lit->where = SAME;
            rel = BELLOW;
        }
        else lit->where = rel;
    }
}

BRES buffer_scroll(buffer* b, CMOVE_DIR dir)
{
    if(dir == UP){
        if(b->bottom->prev && b->top->prev && b->own_curs->own_line != b->bottom && b->own_curs->own_line != b->bottom->prev){
            b->bottom->on_screen = 0;
            b->bottom = b->bottom->prev;

            b->top = b->top->prev;
            b->top->on_screen = 1;
        } 
    }
    if(dir == DOWN){
        if(b->top->next && b->bottom->next && b->own_curs->own_line != b->top && b->own_curs->own_line != b->top->next){
            b->top->on_screen = 0;
            b->top = b->top->next;

            b->bottom = b->bottom->next;
            b->bottom->on_screen = 1;
        } 
    }
}

// should be used to trim the visible area, when l line was inserted
BRES buffer_trim(buffer* b, line* l)
{
    if(!l->on_screen) return FAILED;

    if(l->where == SAME || l->where == BELLOW)
    {
        if(b->bottom != b->own_curs->own_line)
        {
            b->bottom->on_screen = 0;
            b->bottom = b->bottom->prev; 
        }
		else
		{
            b->top->on_screen = 0;
            b->top = b->top->next;
		}
    }
	else if(l->where == ABOVE)
    {
        if(b->top != b->own_curs->own_line)
        {
            b->top->on_screen = 0;
            b->top = b->top->next;
        }
		else 
		{
            b->bottom->on_screen = 0;
            b->bottom = b->bottom->prev; 
		}
    }

    if(b->bottom == b->own_curs->own_line) buffer_scroll(b, DOWN);
    if(b->top == b->own_curs->own_line) buffer_scroll(b, UP);
    
    return UPDATE;
}

// after deleting a line, there is space for one more
BRES buffer_extend(buffer* b, line* l){
    if(l->where == SAME || l->where == BELLOW)
    {
        if(b->bottom->next)
        {
            b->bottom = b->bottom->next; 
            b->bottom->on_screen = 1;
        }
    }
	else if(l->where == ABOVE)
    {
        if(b->top->prev)
        {
            b->top = b->top->prev;
            b->top->on_screen = 1;
        }
        else
        {
            b->bottom = b->bottom->next; 
            b->bottom->on_screen = 1;
        }
    }

    if(b->bottom == b->own_curs->own_line) buffer_scroll(b, DOWN);
    if(b->top == b->own_curs->own_line) buffer_scroll(b, UP);

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

    if(prev && next)
    {
        if(prev->on_screen == next->on_screen) l->on_screen = prev->on_screen;
        else
        {
            // these conditionals are not needed, but i'll leave them here to record the line of thought behind the decision
            if(prev->on_screen && !next->on_screen) l->on_screen = 0;
            if(!prev->on_screen && next->on_screen) l->on_screen = 0;
        }

        if(prev->where == SAME || prev->where == BELLOW) l->where = BELLOW;
        if(next->where == SAME || next->where == ABOVE) l->where = ABOVE;
    }
    else if(prev)
    {
        l->on_screen = 0;
        l->where = BELLOW;
    }
    else if(next)
    {
        l->on_screen = 0;
        l->where = ABOVE;
    }

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
cursor* cursor_new(int id, buffer* buf, line* l, int pos){
    cursor* c = malloc(sizeof(cursor));

    c->id = id;
    c->buf = buf;
    c->own_line = l;
    c->pos = pos;
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
BRES cursor_insert(cursor* c, char chr){
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

// delete character at cursor location (deleting endline not handled here)
BRES cursor_del(cursor* c){
    if(c->pos == 0) return FAILED;

//    if( rope_char_count(c->own_line->str) != 1 && rope_char_count(c->own_line->str) % c->buf->width == 1 ){
//        buffer_scroll(c->buf, UP);
//        buffer_extend(c->buf, c->own_line);
//    }

    rope_del(c->own_line->str, c->pos-1, 1);
    c->pos--;
    return UPDATE;
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
    else for(int i=0; i<MAX_CURSOR_NUM; i++)
	{
        if(b->peer_curss[i] && b->peer_curss[i]->id == id) return b->peer_curss[i];
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

    binit_onscreen_info(b);
    if(ui)
	{
		b->u = ui_init(b);
		b->server_mode = 0;
	}
    else
	{
		b->u = NULL;
		b->server_mode = 1;
	}

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

	b->server_mode = 0;

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
    
    binit_onscreen_info(b);

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
    else
	{
		b->u = NULL;
		b->server_mode = 1;
	}

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
    while(lit)
    {
        int len = rope_write_cstr(lit->str, b->sp);
        write(fd, b->sp, len-1);
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

// delete line
BRES buffer_deletel(buffer* b, line* l){
    if(l->on_screen) buffer_extend(b, l);

	// these shouldn't be necessary because of buffer_extend, but just to be sure... 
	if(l == b->top)
	{
		b->top = l->prev;
		b->top->on_screen = 1;
	}
	if(l == b->bottom)
	{
		b->bottom = l->next;
		b->bottom->on_screen = 1;
	}
	if(l == b->last) b->last = l->prev;

    if(l->prev)
    {
        if(b->own_curs->own_line == l)
            bcursor_move(b, b->own_curs->id, UP);
        for(int i=0; i<MAX_CURSOR_NUM; i++)
        {
            if(!b->peer_curss[i]) continue;
            if(b->peer_curss[i]->own_line == l)
                bcursor_move(b, b->peer_curss[i]->id, UP);
        }
    }

    line_free(l);

    return UPDATE;
}

// create cursor with id in lid line at pos position
BRES bcursor_new(buffer* b, int id, int lid, int pos){

    // find line
    line* l = bline_find(b, lid);
    cursor* c = cursor_new(id, b, l, pos);
    int seen = l->on_screen;

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
        if(i == MAX_CURSOR_NUM-1) return FAILED;
    }

    if(b->u && seen) ui_update(b->u);
    return UPDATE;
}

BRES bcursor_copy_own(buffer* b, int id)
{
    return bcursor_new(b, id, b->own_curs->own_line->id, b->own_curs->pos);
}

BRES bcursor_free(buffer* b, int id)
{
    cursor* c = bcursor_find(b, id);

    // shouldn't ever delete own cursor
    if(c == b->own_curs) return FAILED;

    int seen = c->own_line->on_screen; 
    for(int i=0; i<MAX_CURSOR_NUM; i++)
    {
        if(c == b->peer_curss[i])
        {
            b->peer_curss[i] = NULL;
            break;
        }
    }

    cursor_free(c);
    if(b->u && seen) ui_update(b->u);

    return UPDATE;
}

// move cursor
// check if we have to scroll or show a new cursor on screen
BRES bcursor_move(buffer* b, int id, CMOVE_DIR dir){

    cursor* c = bcursor_find(b, id);

    int seen = c->own_line->on_screen;
    if(c == b->own_curs)
	{
		if(b->top == c->own_line) buffer_scroll(b, UP);
		if(b->bottom== c->own_line) buffer_scroll(b, DOWN);
	}

    CMOVE_RES res = cursor_move(c, dir);
    if(c->own_line->on_screen) seen = 1;

    if(c == b->own_curs)
    {
        if(res == aUP && b->top == c->own_line) buffer_scroll(b, UP);
        if(res == aDOWN && b->bottom== c->own_line) buffer_scroll(b, DOWN);

        if(res == aUP)
        {
            c->own_line->next->where = BELLOW;
            c->own_line->where = SAME;
        }
        if(res == aDOWN)
        {
            c->own_line->prev->where = ABOVE;
            c->own_line->where = SAME;
        }
    }

    if(b->u && seen) ui_update(b->u);
    return UPDATE;
}

// isnert character at cursor location
BRES bcursor_insert(buffer* b, int id, char chr){
    
    cursor* c = bcursor_find(b, id);
	if(!c) return FAILED;
    BRES res = cursor_insert(c, chr);

    if(res == UPDATE || res == SUCCESS)
    {
        if(c != b->own_curs && c->own_line == b->own_curs->own_line && c->pos < b->own_curs->pos)
            b->own_curs->pos++;
        for(int i=0; i<MAX_CURSOR_NUM; i++)
        {
            if(!b->peer_curss[i]) continue;
            if(c != b->peer_curss[i] && c->own_line == b->peer_curss[i]->own_line && c->pos < b->peer_curss[i]->pos)
                b->peer_curss[i]->pos++;
        }
    }

    if(b->u) ui_update(b->u);
    return UPDATE;
}


// isnert new line at cursor location
// if cursor is at pos 0, it will stay in the line and the new line will be above the current line
// if the cursor is somewhere else, it will be moved to pos 0 in the new line, and the contents to the right of the cursor will be moved into the new line
BRES bcursor_insert_line(buffer* b, int id){
    
    b->num_lines++;
    cursor* c = bcursor_find(b, id);

    if(c->pos == 0)
	{
        line* ll = line_new(b->line_id_cnt++, c->own_line->prev, c->own_line);
        if(c->own_line == b->first) b->first = ll;

        if(ll->on_screen)
        {
            buffer_trim(b, ll);
        }
		if(c == b->own_curs) buffer_scroll(b, UP);

		if(b->server_mode && b->own_curs->own_line != b->first) bcursor_move(b, b->own_curs->id, UP);

        if(b->u && ll->on_screen) ui_update(b->u);
        return UPDATE;
    }

    line* ll = line_new(b->line_id_cnt++, c->own_line, c->own_line->next);
    if(c->own_line == b->last) b->last = ll;

    if(c->pos != rope_char_count(c->own_line->str))
    {
        rope_free(ll->str);
        ll->str = rope_copy(ll->prev->str);
        rope_del(ll->prev->str, c->pos, rope_char_count(ll->str)-c->pos);
        rope_del(ll->str, 0, c->pos);
        
        if(c != b->own_curs && c->own_line == b->own_curs->own_line && c->pos < b->own_curs->pos)
        {
            b->own_curs->pos -= c->pos;
            bcursor_move(b, b->own_curs->id, DOWN);
        }
        for(int i=0; i<MAX_CURSOR_NUM; i++)
        {
            if(!b->peer_curss[i]) continue;
            if(c != b->peer_curss[i] && c->own_line == b->peer_curss[i]->own_line && c->pos < b->peer_curss[i]->pos)
            {
                b->peer_curss[i]->pos -= c->pos;
                bcursor_move(b, b->peer_curss[i]->id, DOWN);
            }
        }

    }

    c->pos = 0;

    if(ll->on_screen)
	{
		buffer_trim(b, ll);
		if(c == b->own_curs) buffer_scroll(b, UP);
	}

    //c->own_line = c->own_line->next;
    bcursor_move(b, id, DOWN);

    if(b->u && ll->on_screen) ui_update(b->u);
    return UPDATE;
}

// delete character at cursor location
BRES bcursor_del(buffer* b, int id){
    
    cursor* c = bcursor_find(b, id);
    int seen = c->own_line->on_screen;
    if(c->pos > 0)
    {
        BRES res = cursor_del(c);
        if(res == UPDATE || res == SUCCESS)
        {
            if(c != b->own_curs && c->own_line == b->own_curs->own_line && c->pos < b->own_curs->pos)
                b->own_curs->pos--;
            for(int i=0; i<MAX_CURSOR_NUM; i++)
            {
                if(!b->peer_curss[i]) continue;
                if(c != b->peer_curss[i] && c->own_line == b->peer_curss[i]->own_line && c->pos < b->peer_curss[i]->pos)
                    b->peer_curss[i]->pos--;
            }
        }
    }
    else
    {
        if(c->own_line == b->first) return FAILED;
        b->num_lines--;

        // move deleting cursor into prev line
        bcursor_move(b, id, LEFT);

        // join contents of the two lines
        if(c->own_line->next == b->last) b->last = c->own_line;
        if( rope_write_cstr(c->own_line->next->str, b->sp)>1 ){
            rope_insert(c->own_line->str, rope_char_count(c->own_line->str), b->sp);
        }

        // take care of peer cursors
        if(c != b->own_curs && c->own_line->next == b->own_curs->own_line)
        {
            bcursor_move(b, b->own_curs->id, UP);
            b->own_curs->pos += c->pos;
        }
        for(int i=0; i<MAX_CURSOR_NUM; i++)
        {
            if(!b->peer_curss[i]) continue;
            if(c != b->peer_curss[i] && c->own_line->next == b->peer_curss[i]->own_line)
            {
                bcursor_move(b, b->peer_curss[i]->id, UP);
                b->peer_curss[i]->pos += c->pos;
            }
        }

        // delete line
        buffer_deletel(b, c->own_line->next);
    }

    if(b->u && (seen || c->own_line->on_screen)) ui_update(b->u);
    return UPDATE;
}

void buffer_add_ui(buffer* b)
{
    // already has ui
    if(b->u) return;

    b->u = ui_init(b);
    ui_update(b->u);
    return;
}

buffer* buffer_deserialize(char* serd, int u)
{
    
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
	b->server_mode = 0;

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

    binit_onscreen_info(b);

    if(u)
    {
        b->u = ui_init(b);
        ui_update(b->u);
		b->server_mode = 0;
    }
    else
	{
		b->u = NULL;
		b->server_mode = 1;
	}

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

    for(int i=0; i<MAX_CURSOR_NUM; i++){
        if(c = b->peer_curss[i]){
            // own_line
            snprintf(path, 2048, "peer_curss.%d.own_line", c->id);
            json_object_dotset_number(root_object, path, c->own_line->id);
            // position
            snprintf(path, 2048, "peer_curss.%d.pos", c->id);
            json_object_dotset_number(root_object, path, c->pos);
        }
    }

    serialized_string = json_serialize_to_string(root_value);
    //puts(serialized_string);
    //printf("%d", sizeof(serialized_string));
    //json_free_serialized_string(serialized_string);
    json_value_free(root_value);
    return serialized_string;
}
