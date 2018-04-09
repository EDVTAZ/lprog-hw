#include <ncurses.h>
#include <rope.h>
#include <stdlib.h>

#include <buffer.h>
//#include "../include/buffer.h"
#include <ui.h>

#define HEIGHT 80
#define WIDTH 80

int main()
{

    buffer* b = buffer_new(0, 0, 0, HEIGHT, WIDTH);

    int c;
    while(c != KEY_F(8)){
        c = getch();

        switch(c){
            case KEY_LEFT:
                bcursor_move(b, 0, LEFT);
                break;
                
            case KEY_RIGHT:
                bcursor_move(b, 0, RIGHT);
                break;

            case KEY_UP:
                bcursor_move(b, 0, UP);
                break;

            case KEY_DOWN:
                bcursor_move(b, 0, DOWN);
                break;

            case KEY_BACKSPACE:
                bcursor_del(b, 0);
                break;

            case KEY_F(8):
                break;

            case '\n':
                bcursor_insert_line(b, 0);
                break;

            default:
                bcursor_insert(b, 0, c);
        }
    }


    endwin();                /* End curses mode        */

    return 0;
}
