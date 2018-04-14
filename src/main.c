#include <ncurses.h>
#include <rope.h>
#include <stdlib.h>

#include <buffer.h>
//#include "../include/buffer.h"
#include <ui.h>

#define HEIGHT 20
#define WIDTH 20

int main()
{

    //buffer* b = buffer_new(0, 0, HEIGHT, WIDTH, 1);
    buffer* b = buffer_from_file("testfile", 0, HEIGHT, WIDTH, 1);
    char** data;
    int size;

    ui_update(b->u);

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
                buffer_save("atestfile", b);
                buffer_free(b);
                break;

            case KEY_F(7):

                data = malloc(sizeof(char*));
                size = buffer_serialize(b, data);

                buffer_free(b);

                b = buffer_deserialize(*data, size);
                free(*data);
                free(data);

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
