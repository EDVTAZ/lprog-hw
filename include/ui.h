#ifndef ui_h
#define ui_h

#include <ncurses.h>
#include <buffer.h>

typedef struct ui ui;

struct ui{
    
    // ncurses window
    WINDOW* win;
    // data backend
    struct buffer* buf;

    // dimensions
    int height;
    int width;

};

// initialize ui
ui* ui_init(struct buffer* b);

// free ui
void ui_free(ui* u);

// update everything
void ui_update(ui* u);
/*
// update the location of a cursor
void ui_curs_update(ui* u, struct cursor* c);

// update after a deletion
void ui_del_update(ui* u, struct cursor* c);

// update after an insertion
void ui_ins_update(ui* u, struct cursor* c);

// update after a new line was inserted
void ui_nline_update(ui* u, struct line* l);

// update after a line was deleted
void ui_dline_update(ui* u, struct line* l);
*/

#endif
