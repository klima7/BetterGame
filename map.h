#ifndef __MAP_H__
#define __MAP_H__

#include <stdint.h>
#include <ncursesw/ncurses.h>
#include "common.h"

#define MAP_VISIBLE_DISTANCE 2

enum tile_t
{
    TILE_VOID       = 0,
    TILE_WALL       = 1,
    TILE_FLOOR      = 2,
    TILE_CAMPSICE   = 3,
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

struct map_t
{
    int viewpoint_x;
    int viewpoint_y;

    enum tile_t map[MAP_HEIGHT][MAP_WIDTH];
};

void map_move_tile(struct map_t *map, int src_x, int src_y, int dst_x, int dst_y);

const chtype map_get_color_char_from_tile(enum tile_t tile);
void map_display(struct map_t *map, WINDOW *window);
enum tile_t map_get_tile(struct map_t *map, int x, int y);

#endif