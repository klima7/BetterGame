#ifndef __TILES_H__
#define __TILES_H__

#include <ncursesw/ncurses.h>

// Kafelki z których składa się mapa
enum tile_t
{
    TILE_VOID       = 0,
    TILE_WALL       = 1,
    TILE_FLOOR      = 2,
    TILE_CAMPSIDE   = 3,
    TILE_BUSH       = 4,
    TILE_BEAST      = 5,
    TILE_COIN       = 6,
    TILE_S_TREASURE = 7,
    TILE_L_TREASURE = 8,
    TILE_DROP       = 9,
    TILE_UNKNOWN    = 10,

    TILE_PLAYER1    = 11,
    TILE_PLAYER2    = 12,
    TILE_PLAYER3    = 13,
    TILE_PLAYER4    = 14
};

int tile_is_sure(tile_t tile);
int tile_is_walkable(tile_t tile);
int tile_is_player(tile_t tile);
const chtype tile_get_appearance(enum tile_t tile);
void display_help_window(WINDOW *window);

#endif