#ifndef __MAP_H__
#define __MAP_H__

#include <stdint.h>
#include <ncursesw/ncurses.h>
#include "common.h"
#include "tiles.h"

// Stałe używane przy generacji labiryntu
#define MAP_GEN_UP 0
#define MAP_GEN_DOWN 1
#define MAP_GEN_LEFT 2
#define MAP_GEN_RIGHT 3

// Współczynniki mówiące o liczbe danych przedmiotów na mapie count=MAP_WIDTH*MAP_HEIGHT/FACTOR
#define MAP_GEN_BUSH_FACTOR 20
#define MAP_GEN_COIN_FACTOR 80
#define MAP_GEN_TREASURE_S_FACTOR 80
#define MAP_GEN_TREASURE_L_FACTOR 80
#define MAP_GEN_BEAST_FACTOR 300
#define MAP_GEN_HOLES_FACTOR 50

// Mapa
struct map_t
{
    int viewpoint_x;
    int viewpoint_y;

    int campside_x;
    int campside_y;

    enum tile_t map[MAP_HEIGHT][MAP_WIDTH];
};

// Prototypy
void map_display(struct map_t *map, WINDOW *window);
enum tile_t map_get_tile(struct map_t *map, int x, int y);
void map_set_tile(struct map_t *map, int x, int y, enum tile_t tile);
void map_copy(const struct map_t *source, struct map_t *destination);
void map_fill(struct map_t *map, enum tile_t tile);
void map_update_with_surrounding_area(struct map_t *map, surrounding_area_t *area, int x, int y);
void map_remove_unsure_tiles(struct map_t *map);
void map_generate_maze(struct map_t *map);
void map_shift(struct map_t *map, int shift_x, int shift_y);
int map_random_free_position(struct map_t *map, int *resx, int *resy);
void map_generate_everything(struct map_t *map);

#endif