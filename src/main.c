#include<ncurses.h>

#define HEIGHT 20
#define WIDTH 20


int main()
{
    initscr();               /* Start curses mode          */
    //raw();                  // line buffering disabled
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(1);


    //printw("Hello World !!!");  /* Print Hello World          */
    //refresh();               /* Print it on to the real screen */
    //getch();                 /* Wait for user input */
    //mvprintw(0, 1, "Hello World !!!");  /* Print Hello World          */
    //refresh();               /* Print it on to the real screen */
    //getch();                 /* Wait for user input */
    
    char s[2];
    s[1] = 0;
    int pozX = 0;
    int pozY = 0;
    int c=0;
    char buf[20][20];
    while(c != KEY_F(8)){
        c = getch();

        switch(c){
            case KEY_LEFT:
                if(pozX > 0) pozX--;
                break;
                
            case KEY_RIGHT:
                if(pozX < WIDTH-1) pozX++;
                else if(pozY < HEIGHT-1){
                    pozX = 0;
                    pozY++;
                }
                break;

            case KEY_UP:
                if(pozY > 0) pozY--;
                break;

            case KEY_DOWN:
                if(pozY < HEIGHT-1) pozY++;
                break;

            case KEY_BACKSPACE:
                if(pozX > 0){
                    pozX--;
                    buf[pozX][pozY]=0;
                }
                else if(pozY > 0){
                    pozY--;
                    pozX = WIDTH-1;
                    buf[pozX][pozY]=0;
                }

            case KEY_F(8):
                break;

            default:
                buf[pozX][pozY] = c;
                s[0] = c;
                mvprintw(pozY, pozX, s);
                s[0] = buf[pozX][pozY];
                mvprintw(HEIGHT+1, WIDTH+1, s);
                if(pozX < WIDTH-1) pozX++;
                else if(pozY < HEIGHT-1){
                    pozX = 0;
                    pozY++;
                }
        }
        move(pozY, pozX);
        refresh();
    }


    endwin();                /* End curses mode        */

    return 0;
}
