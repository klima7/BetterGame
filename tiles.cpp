#include <ncursesw/ncurses.h>
#include "tiles.h"
#include "common.h"

// Wygląd przypisany poszczególnym kafelkom
const chtype associated_appearance[] =
{
    ' ' | COLOR_PAIR(COLOR_BLACK_ON_WHITE),
    ' ' | COLOR_PAIR(COLOR_WHITE_ON_BLACK),
    ' ' | COLOR_PAIR(COLOR_BLACK_ON_WHITE),
    'A' | COLOR_PAIR(COLOR_YELLOW_ON_GREEN),
    '#' | COLOR_PAIR(COLOR_BLACK_ON_WHITE),
    '*' | COLOR_PAIR(COLOR_RED_ON_WHITE),
    'c' | COLOR_PAIR(COLOR_BLACK_ON_YELLOW),
    't' | COLOR_PAIR(COLOR_BLACK_ON_YELLOW),
    'T' | COLOR_PAIR(COLOR_BLACK_ON_YELLOW),
    'D' | COLOR_PAIR(COLOR_GREEN_ON_YELLOW),
    '.' | COLOR_PAIR(COLOR_BLACK_ON_WHITE),

    '1' | COLOR_PAIR(COLOR_WHITE_ON_MAGENTA),
    '2' | COLOR_PAIR(COLOR_WHITE_ON_MAGENTA),
    '3' | COLOR_PAIR(COLOR_WHITE_ON_MAGENTA),
    '4' | COLOR_PAIR(COLOR_WHITE_ON_MAGENTA)
};

// Czy kafelek jest pewny - po zniknięciu z pola widzenia nie znika
int tile_is_sure(tile_t tile)
{
    if(tile==TILE_WALL || tile==TILE_FLOOR || tile==TILE_CAMPSIDE || tile==TILE_BUSH || tile==TILE_DROP || tile==TILE_UNKNOWN) return 1;
    return 0;
}

// Czy po kafelku można chodzić
int tile_is_walkable(tile_t tile)
{
    if(tile==TILE_FLOOR || tile==TILE_L_TREASURE || tile==TILE_S_TREASURE || tile==TILE_COIN || tile==TILE_DROP || tile==TILE_UNKNOWN || tile==TILE_CAMPSIDE || tile==TILE_BUSH) return 1;
    if(tile_is_player(tile)) return 1;
    else return 0;
}

// Czy kafelek jest graczem
int tile_is_player(tile_t tile)
{
    if(tile==TILE_PLAYER1 || tile==TILE_PLAYER2 || tile==TILE_PLAYER3 || tile==TILE_PLAYER4) return 1;
    else return 0;
}

// Zwraca wygląd danego kafelka
const chtype tile_get_appearance(enum tile_t tile)
{
    int index = (int)tile;
    return associated_appearance[index];
}

// Wyświetla w oknie wyjaśnienia do gry
void display_help_window(WINDOW *window)
{
    werase(window);
    int line = 0;

    wattron(window, COLOR_PAIR(COLOR_WHITE_ON_RED));
    mvwprintw(window, line++, 0, "Legend:");
    wattron(window, COLOR_PAIR(COLOR_BLACK_ON_WHITE));

    line++;

    enum tile_t tiles[] = {TILE_PLAYER1, TILE_PLAYER2, TILE_PLAYER3, TILE_PLAYER4, 
    TILE_COIN, TILE_S_TREASURE, TILE_L_TREASURE, TILE_CAMPSIDE, TILE_DROP, TILE_WALL, TILE_UNKNOWN, TILE_BUSH, TILE_BEAST };

    const char *names[] = {"Player 1", "Player 2", "Player 3", "Player 4",
    "Coint", "Small Treasure", "Large Treasure", "Campside", "Dropped Treasure", "Wall", "Unknown", "Bush", "Beast"};

    mvwaddch(window, line, 1, tile_get_appearance(TILE_PLAYER1));
    mvwprintw(window, line++, 5, "Player 1");

    for(int i=0; i<13; i++)
    {
        mvwaddch(window, line, 1, tile_get_appearance(tiles[i]));
        mvwprintw(window, line++, 5, names[i]);
    }

    line++;

    wattron(window, COLOR_PAIR(COLOR_WHITE_ON_RED));
    mvwprintw(window, line++, 0, "By Lukasz Klimkiewicz");
    wattron(window, COLOR_PAIR(COLOR_BLACK_ON_WHITE));

    wrefresh(window);
}