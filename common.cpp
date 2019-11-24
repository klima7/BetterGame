#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "ncursesw/ncurses.h"

void check(int expr, const char *message)
{
    if(!expr)
    {
        display_center(message);
        getchar();
        endwin();
        exit(1);
    }
}

void display_center(const char *message)
{
        int w = getmaxx(stdscr);
        int h = getmaxy(stdscr);
        clear();
        mvprintw(h/2, (w-strlen(message))/2, message);
        refresh();
}

void init_colors(void)
{
    start_color();

    init_pair(COLOR_WHITE_ON_BLACK, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_BLACK_ON_WHITE, COLOR_BLACK, COLOR_WHITE);
    init_pair(COLOR_RED_ON_WHITE, COLOR_RED, COLOR_WHITE);
    init_pair(COLOR_BLACK_ON_YELLOW, COLOR_BLACK, COLOR_YELLOW);
    init_pair(COLOR_WHITE_ON_MAGENTA, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(COLOR_GREEN_ON_YELLOW, COLOR_GREEN, COLOR_YELLOW);
    init_pair(COLOR_YELLOW_ON_GREEN, COLOR_YELLOW, COLOR_GREEN);
    init_pair(COLOR_WHITE_ON_RED, COLOR_WHITE, COLOR_RED);
}

enum action_t reverse_direction(enum action_t direction)
{
    if(direction==ACTION_GO_DOWN) return ACTION_GO_UP;
    else if(direction==ACTION_GO_UP) return ACTION_GO_DOWN;
    else if(direction==ACTION_GO_LEFT) return ACTION_GO_RIGHT;
    else if(direction==ACTION_GO_RIGHT) return ACTION_GO_LEFT;
    return direction;
}