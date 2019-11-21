#ifndef __MAP_H__
#define __MAP_H__

#include <stdint.h>
#include <ncursesw/ncurses.h>
#include "common.h"

#define MAP_GEN_UP 0
#define MAP_GEN_DOWN 1
#define MAP_GEN_LEFT 2
#define MAP_GEN_RIGHT 3

struct map_t
{
    int viewpoint_x;
    int viewpoint_y;

    int campside_x;
    int campside_y;

    enum tile_t map[MAP_HEIGHT][MAP_WIDTH];
};

int map_is_sure_tile(tile_t tile);
int map_is_walkable_tile(tile_t tile);
int map_is_player_tile(tile_t tile);
const chtype map_get_color_char_from_tile(enum tile_t tile);
void map_display(struct map_t *map, WINDOW *window);
enum tile_t map_get_tile(struct map_t *map, int x, int y);
void map_set_tile(struct map_t *map, int x, int y, enum tile_t tile);
void map_copy(const struct map_t *source, struct map_t *destination);
void map_fill(struct map_t *map, enum tile_t tile);
void map_update_with_surrounding_area(struct map_t *map, surrounding_area_t *area, int x, int y);
void map_remove_unsure_tiles(struct map_t *map);
void map_generate_maze(struct map_t *map);
void map_shift(struct map_t *map, int shift_x, int shift_y);

#endif