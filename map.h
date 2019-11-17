#ifndef __MAP_H__
#define __MAP_H__

#include <stdint.h>
#include <ncursesw/ncurses.h>
#include "common.h"

struct map_t
{
    int viewpoint_x;
    int viewpoint_y;

    enum tile_t map[MAP_HEIGHT][MAP_WIDTH];
};

const chtype map_get_color_char_from_tile(enum tile_t tile);
void map_display(struct map_t *map, WINDOW *window);
enum tile_t map_get_tile(struct map_t *map, int x, int y);
void map_set_tile(struct map_t *map, int x, int y, enum tile_t tile);
void map_copy(const struct map_t *source, struct map_t *destination);
void map_set_unknown(struct map_t *map);
void map_update_with_surrounding_area(struct map_t *map, surrounding_area_t *area, int x, int y);

#endif