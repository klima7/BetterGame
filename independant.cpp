#include <stdlib.h>
#include "independant.h"
#include "common.h"
#include "map.h"
#include "tiles.h"

// Funkcje statyczne
static enum action_t indep_navigate_tile_recursion(struct map_t *map, int sx, int sy, enum tile_t dst, int distance);

// Funkcja znajdująca najkrótszą drogę z danego punktu do najgliższego kafelka dst, ale nie dłuższą niż distance
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

// Funkcja rekurencyjna znajdująca drogę
static enum action_t indep_navigate_tile_recursion(struct map_t *map, int sx, int sy, enum tile_t dst, int distance)
{
    if(map_get_tile(map, sx, sy)==dst) 
        return ACTION_DO_NOTHING;

    map_set_tile(map, sx, sy, TILE_WALL);

    distance--;
    if(distance<0)
        return ACTION_VOID;

    enum action_t results[4] = { ACTION_VOID, ACTION_VOID, ACTION_VOID, ACTION_VOID };

    if(tile_is_walkable(map_get_tile(map, sx-1, sy)) || map_get_tile(map, sx-1, sy)==dst) results[0] = indep_navigate_tile_recursion(map, sx-1, sy, dst, distance);
    if(tile_is_walkable(map_get_tile(map, sx+1, sy)) || map_get_tile(map, sx+1, sy)==dst) results[1] = indep_navigate_tile_recursion(map, sx+1, sy, dst, distance);
    if(tile_is_walkable(map_get_tile(map, sx, sy-1)) || map_get_tile(map, sx, sy-1)==dst) results[2] = indep_navigate_tile_recursion(map, sx, sy-1, dst, distance);
    if(tile_is_walkable(map_get_tile(map, sx, sy+1)) || map_get_tile(map, sx, sy+1)==dst) results[3] = indep_navigate_tile_recursion(map, sx, sy+1, dst, distance);

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

// W którą stroną powinien pójść gracz, aby podążać lewą ścianą
action_t indep_follow_left_wall(struct map_t *map, int x, int y, action_t current_direction)
{
    if(current_direction==ACTION_DO_NOTHING)
        current_direction =  ACTION_GO_LEFT;   

    for(int i=0; i<4; i++)
    {

        int left_x = x;
        int left_y = y;

        if(current_direction==ACTION_GO_UP)
            left_x--;
        else if(current_direction==ACTION_GO_DOWN)
            left_x++;
        else if(current_direction==ACTION_GO_LEFT)
            left_y++;
        else if(current_direction==ACTION_GO_RIGHT)
            left_y--;

        enum tile_t left_tile = map_get_tile(map, left_x, left_y);

        if(tile_is_walkable(left_tile))
        {
            if(current_direction==ACTION_GO_UP)
                return ACTION_GO_LEFT;
            else if(current_direction==ACTION_GO_DOWN)
                return ACTION_GO_RIGHT;
            else if(current_direction==ACTION_GO_LEFT)
                return ACTION_GO_DOWN;
            else if(current_direction==ACTION_GO_RIGHT)
                return ACTION_GO_UP;   
        }

        if(current_direction==ACTION_GO_UP)
            current_direction = ACTION_GO_RIGHT;
        else if(current_direction==ACTION_GO_DOWN)
            current_direction = ACTION_GO_LEFT;
        else if(current_direction==ACTION_GO_LEFT)
            current_direction = ACTION_GO_UP;
        else if(current_direction==ACTION_GO_RIGHT)
            current_direction = ACTION_GO_DOWN;   
    }
    
    return ACTION_DO_NOTHING;
}