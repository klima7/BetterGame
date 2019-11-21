#include <stdlib.h>
#include "independant.h"
#include "common.h"
#include "map.h"

// Funkcje statyczne
static enum action_t indep_navigate_tile_recursion(struct map_t *map, int sx, int sy, enum tile_t dst, int distance);

enum action_t indep_navigate_tile(struct map_t *map, int sx, int sy, enum tile_t dst, int distance)
{
    for(int i=0; i<=distance; i++)
    {
        struct map_t copy = *map;
        enum action_t direction = indep_navigate_tile_recursion(&copy, sx, sy, dst, i);
        if(direction!=ACTION_VOID) return direction;
    }
    return ACTION_VOID;
}

static enum action_t indep_navigate_tile_recursion(struct map_t *map, int sx, int sy, enum tile_t dst, int distance)
{
    if(map_get_tile(map, sx, sy)==dst) 
        return ACTION_DO_NOTHING;

    map_set_tile(map, sx, sy, TILE_WALL);

    distance--;
    if(distance<0)
        return ACTION_VOID;

    enum action_t results[4] = { ACTION_VOID, ACTION_VOID, ACTION_VOID, ACTION_VOID };

    if(map_is_walkable_tile(map_get_tile(map, sx-1, sy)) || map_get_tile(map, sx-1, sy)==dst) results[0] = indep_navigate_tile_recursion(map, sx-1, sy, dst, distance);
    if(map_is_walkable_tile(map_get_tile(map, sx+1, sy)) || map_get_tile(map, sx+1, sy)==dst) results[1] = indep_navigate_tile_recursion(map, sx+1, sy, dst, distance);
    if(map_is_walkable_tile(map_get_tile(map, sx, sy-1)) || map_get_tile(map, sx, sy-1)==dst) results[2] = indep_navigate_tile_recursion(map, sx, sy-1, dst, distance);
    if(map_is_walkable_tile(map_get_tile(map, sx, sy+1)) || map_get_tile(map, sx, sy+1)==dst) results[3] = indep_navigate_tile_recursion(map, sx, sy+1, dst, distance);

    int possible_ways = 0;

    if(results[0]!=ACTION_VOID) possible_ways++;
    if(results[1]!=ACTION_VOID) possible_ways++;
    if(results[2]!=ACTION_VOID) possible_ways++;
    if(results[3]!=ACTION_VOID) possible_ways++;

    if(possible_ways==0) 
        return ACTION_VOID;

    int way = rand()%possible_ways;

    for(int i=0; i<4; i++)
    {
        if(results[i]!=ACTION_VOID) 
        {
            if(way==0) 
            {
                if(i==0) return ACTION_GO_LEFT;
                else if(i==1) return ACTION_GO_RIGHT;
                else if(i==2) return ACTION_GO_UP;
                else if(i==3) return ACTION_GO_DOWN;
            }
            way--;
        }
    }
    return ACTION_VOID;
}