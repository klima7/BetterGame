#include <pthread.h>
#include <ncursesw/ncurses.h>
#include <stdlib.h>
#include "common.h"
#include "beast.h"
#include "independant.h"
#include "server_data.h"
#include "map.h"

int beast_see_player(struct beast_t *beast, struct map_t *map)
{
    int x = beast->x;
    int y = beast->y;

    for(int i=-1; i<=1; i++)
    {
        for(int j=-1; j<=1; j++)
        {
            tile_t tile = map_get_tile(map, x+i, y+j);
            if(map_is_player_tile(tile)) return 1;
        }
    }

    for(int ay=-1; ay<=1; ay+=2)
    {
        for(int ax=-1; ax<=1; ax+=2)
        {
            if(map_is_player_tile(map_get_tile(map, x+2*ax, y+2*ay)) &&
            map_get_tile(map, x+1*ax, y+1*ay)!=TILE_WALL) 
                return 1;
            

            if(map_is_player_tile(map_get_tile(map, x+2*ax, y)) &&
            map_get_tile(map, x+1*ax, y)!=TILE_WALL) 
                return 1;

            if(map_is_player_tile(map_get_tile(map, x, y+2*ay)) &&
            map_get_tile(map, x, y+1*ay)!=TILE_WALL) 
                return 1;

            if(map_is_player_tile(map_get_tile(map, x+2*ax, y+1*ay)) &&
            map_get_tile(map, x+1*ax, y+1*ay)!=TILE_WALL && map_get_tile(map, x+1*ax, y)!=TILE_WALL) 
                return 1;

            if(map_is_player_tile(map_get_tile(map, x+1*ax, y+2*ay)) &&
            map_get_tile(map, x+1*ax, y+1*ay)!=TILE_WALL && map_get_tile(map, x, y+1*ay)!=TILE_WALL) 
                return 1;
        }
    }

    return 0;
}

void beast_init(struct beast_t *beast, int x, int y)
{
    beast->x = x;
    beast->y = y;
    beast->current_direction = (enum action_t)(rand()%4);
    beast->turns_to_stay = rand()%BEAST_MAX_STAY_TURNS;
}

void beast_update(struct server_data_t *sd, int nr)
{
    struct beast_t *beast = &sd->beasts.at(nr);

    struct map_t complete_map;
    sd_create_complete_map(sd, &complete_map);

    int current_x = beast->x;
    int current_y = beast->y;

    int next_x = current_x;
    int next_y = current_y;

    if(beast->current_direction==ACTION_GO_DOWN) next_y++;
    else if(beast->current_direction==ACTION_GO_UP) next_y--;
    else if(beast->current_direction==ACTION_GO_LEFT) next_x--;
    else if(beast->current_direction==ACTION_GO_RIGHT) next_x++;

    // Atakuje gracza, jeśli żadna ściana go nie zasłania
    if(beast_see_player(beast, &complete_map))
    {
        enum action_t direction = ACTION_DO_NOTHING;
        if(
        (direction=indep_navigate_tile(&complete_map, beast->x, beast->y, TILE_PLAYER1, 3))!=ACTION_VOID ||
        (direction=indep_navigate_tile(&complete_map, beast->x, beast->y, TILE_PLAYER2, 3))!=ACTION_VOID ||
        (direction=indep_navigate_tile(&complete_map, beast->x, beast->y, TILE_PLAYER3, 3))!=ACTION_VOID ||
        (direction=indep_navigate_tile(&complete_map, beast->x, beast->y, TILE_PLAYER4, 3))!=ACTION_VOID)
        if(direction!=ACTION_VOID)
        {
            beast->current_direction = direction;
            sd_move_beast(sd, beast, direction);
        }
        return;
    }

    // Czeka sobie
    if(beast->turns_to_stay>0)
    {
        beast->turns_to_stay--;
        return;
    }

    // Zatrzymuje się na chwile
    int wait_a_moment = rand()%BEAST_STAY_PROBABILITY;
    if(!wait_a_moment)
    {
        beast->turns_to_stay = rand()%BEAST_MAX_STAY_TURNS;
        return;
    }

    // Idz w tą samą stronę w którą idziesz
    if(map_is_walkable_tile(map_get_tile(&complete_map, next_x, next_y)))
    {
        sd_move_beast(sd, beast, beast->current_direction);
        return;
    }

    // Jeśli się z czymś zderzysz to losój nowy kierunek
    enum action_t direction = indep_navigate_tile(&complete_map, beast->x, beast->y, TILE_FLOOR, 1);
    if(direction!=ACTION_VOID)
    {
        sd_move_beast(sd, beast, direction);
        beast->current_direction = direction;
        return;
    }
}