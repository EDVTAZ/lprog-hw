#include <ui.h>
#include <stdlib.h>
//#include "../include/ui.h"

// initialize ui
ui* ui_init(buffer* b){
    ui* u = malloc(sizeof(ui));

    u->buf = b;
    u->height = b->height;
    u->width = b->width;
    //
    // init ncurses
    initscr();
    if(has_colors() == FALSE)
    {   endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    start_color();          /* Start color          */
    init_pair(1, COLOR_BLUE, COLOR_GREEN);
    //raw();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(1);

    return u;
}

// free ui
void ui_free(ui* u){

    endwin();

    free(u);
}

// update everything
void ui_update(ui* u){
    clear();
    char cstr[u->width * u->height + 1];
    line* lit = u->buf->top;
    int ypos, xpos;

    lit = u->buf->first;
    for(int i=0; lit; i++){

        mvprintw(i, 50, "%d", lit->id);

        if(lit->on_screen) printw("\t+");
        else printw("\t-");

        if(lit->where == ABOVE) printw("\tA");
        else if(lit->where == BELLOW) printw("\tB");
        else printw("\tS");

        if(lit->next) lit = lit->next;
        else break;
    }

    lit = u->buf->top;
    for(int i=0; i<u->height; i++){
        if(u->buf->own_curs->own_line == lit){
            ypos = i;
            xpos = u->buf->own_curs->pos;
        }

        rope_write_cstr(lit->str, cstr);
        mvprintw(i, 0, cstr);
        mvprintw(i, 25, "   |%d", lit->id);

        // color peer cursors (for now this overwrites current char at position but w/e)
        for(int j=0; j<MAX_CURSOR_NUM; j++)
        {
            if(u->buf->peer_curss[j] && u->buf->peer_curss[j]->own_line->on_screen)
            {
                if(lit == u->buf->peer_curss[j]->own_line)
                {
                    attron(COLOR_PAIR(1));
                    mvprintw(i, u->buf->peer_curss[j]->pos, " ");
                    attroff(COLOR_PAIR(1));
                }
            }
        }

        if(lit->next) lit = lit->next;
        else break;
    }

    mvprintw(49, 0, "%d %d\n", u->buf->first->id, u->buf->last->id);
    mvprintw(50, 0, "%d %d\n", u->buf->top->id, u->buf->bottom->id);

    move(ypos, xpos);
    refresh();
}

// update the location of a cursor
void ui_curs_update(ui* u, cursor* c){} 

// update after a deletion
void ui_del_update(ui* u, cursor* c){}  

// update after an insertion
void ui_ins_update(ui* u, cursor* c){}  

// update after a new line was inserted
void ui_nline_update(ui* u, line* l){}  

// update after a line was deleted
void ui_dline_update(ui* u, line* l){}  

