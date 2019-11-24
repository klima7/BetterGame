#ifndef __INDEPENDANT_H__
#define __INDEPENDANT_H__

#include "common.h"

enum action_t indep_navigate_tile(struct map_t *map, int sx, int sy, enum tile_t dst, int distance);
action_t indep_go_somewhere_but_not_there(enum action_t not_there, struct map_t *map, int x, int y);
action_t indep_follow_left_wall(struct map_t *map, int x, int y, action_t current_direction);

#endif