#include <ncurses.h>
#include <rope.h>
#include <stdlib.h>

#include <buffer.h>
#include <ui.h>

#define HEIGHT 20
#define WIDTH 20

int main()
{


    //buffer* b = buffer_new(0, 0, HEIGHT, WIDTH, 1);
    buffer* b = buffer_from_file("testfile", 0, HEIGHT, WIDTH, 1);
    bcursor_new(b, 1, 0, 0);
    char* data;
    int size;

    int cid = 0;

    ui_update(b->u);

    int c;
    while(c != KEY_F(8)){
        cid ^= 1;
        c = getch();

        switch(c){
            case KEY_LEFT:
                bcursor_move(b, cid, LEFT);
                break;

            case KEY_RIGHT:
                bcursor_move(b, cid, RIGHT);
                break;

            case KEY_UP:
                bcursor_move(b, cid, UP);
                break;

            case KEY_DOWN:
                bcursor_move(b, cid, DOWN);
                break;

            case KEY_BACKSPACE:
                bcursor_del(b, cid);
                break;

            case KEY_F(8):
                buffer_save("atestfile", b);
                buffer_free(b);
                break;

            case KEY_F(7):

                data = buffer_serialize(b);

                buffer_free(b);

                b = buffer_deserialize(data, 1);
                free(data);

                break;

            case '\n':
                bcursor_insert_line(b, cid);
                break;

            default:
                bcursor_insert(b, cid, c);
        }
    }


    endwin();                /* End curses mode        */


    return 0;
}
