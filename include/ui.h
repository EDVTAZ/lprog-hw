#ifndef ui_h
#define ui_h

#include <ncurses.h>
#include <buffer.h>

typedef struct{
    
    // ncurses window
    WINDOW* win;
    // data backend
    buffer* buf;

    // dimensions
    int height;
    int width;

} ui;

// initialize ui
ui* ui_init(buffer* b);

// free ui
void ui_free(ui* u);

// update everything
void ui_update(ui* u);

// update the location of a cursor
void ui_curs_update(ui* u, cursor* c);

// update after a deletion
void ui_del_update(ui* u, cursor* c);

// update after an insertion
void ui_ins_update(ui* u, cursor* c);

// update after a new line was inserted
void ui_nline_update(ui* u, line* l);

// update after a line was deleted
void ui_dline_update(ui* u, line* l);

#endif
