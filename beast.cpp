#include <pthread.h>
#include <ncursesw/ncurses.h>
#include <stdlib.h>
#include "common.h"
#include "beast.h"
#include "independant.h"
#include "server_data.h"
#include "map.h"

// Inicjuje bestie
void beast_init(struct beast_t *beast, int x, int y)
{
    beast->x = x;
    beast->y = y;
    beast->current_direction = (enum action_t)(rand()%4);
    beast->turns_to_wait = 0;
}

// Sprawdza czy bestia widzi jakiegoś gracza
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

// Zwraca kierunek do atakowania gracza
enum action_t beast_attack_player(struct beast_t *beast, struct map_t *map)
{
    enum tile_t possible_targets[4] = { TILE_PLAYER1, TILE_PLAYER2, TILE_PLAYER3, TILE_PLAYER4 };
    for(int i=0; i<4; i++)
    {
        enum action_t target_direction = indep_navigate_tile(map, beast->x, beast->y, possible_targets[i], 3);
        if(target_direction!=ACTION_VOID) return target_direction;
    }
    return ACTION_VOID;
}

// Aktualizacja bestii
void beast_update(struct server_data_t *sd, int nr)
{
    struct beast_t *beast = &sd->beasts.at(nr);
    struct map_t complete_map;
    sd_create_complete_map(sd, &complete_map);

    // Atakuje gracza jeśli go widzi
    if(beast_see_player(beast, &complete_map))
    {
        enum action_t direction = beast_attack_player(beast, &complete_map);
        sd_move_beast(sd, beast, direction);
        return;
    }

    // Podąża lewą świaną
    enum action_t direction = indep_follow_left_wall(&complete_map, beast->x, beast->y, beast->current_direction);
    if(beast->turns_to_wait==0) beast->current_direction = direction;
    sd_move_beast(sd, beast, direction);
    return;
}