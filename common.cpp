#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "ncursesw/ncurses.h"

void check(int expr, const char *message)
{
    if(!expr)
    {
        int w = getmaxx(stdscr);
        int h = getmaxy(stdscr);
        clear();
        mvprintw(h/2, (w-strlen(message))/2, message);
        refresh();
        getchar();
        endwin();
        exit(1);
    }
}