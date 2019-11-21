#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <ncursesw/ncurses.h>
#include <ctype.h>
#include "client_common.h"
#include "common.h"
#include "client_data.h"
#include "map.h"
#include "independant.h"

#define MONEY_TO_RETURN 100

// Funkcje statyczne
static void *clientb_input_thread(void *ptr);
static void *clientb_update_thread(void *ptr);
static void clientb_behaviour(void);
static action_t clientb_escape(enum action_t beast_dir, struct map_t *map, int x, int y);

pthread_t input_thread;
pthread_t update_thread;

enum action_t current_action;

static void *clientb_input_thread(void *ptr)
{
    while(1)
    {
        int c = getch();
        if(tolower(c)=='q')
            return NULL;
    }
}

static action_t clientb_escape(enum action_t beast_dir, struct map_t *map, int x, int y)
{
    enum action_t ways[] = { ACTION_GO_UP, ACTION_GO_DOWN, ACTION_GO_LEFT, ACTION_GO_RIGHT };
    
    for(int i=0; i<4; i++)
    {
        if(ways[i]==beast_dir)
            ways[i] = ACTION_VOID;
    }

    if(!map_is_walkable_tile(map_get_tile(map, x, y-1))) ways[0] = ACTION_VOID;
    if(!map_is_walkable_tile(map_get_tile(map, x, y+1))) ways[1] = ACTION_VOID;
    if(!map_is_walkable_tile(map_get_tile(map, x-1, y))) ways[2] = ACTION_VOID;
    if(!map_is_walkable_tile(map_get_tile(map, x+1, y))) ways[3] = ACTION_VOID;

    int good_ways = 0;
    for(int i=0; i<4; i++)
    {
        if(ways[i]!=ACTION_VOID)
            good_ways++;
    }

    // :(
    if(good_ways==0)
        return ACTION_DO_NOTHING;

    int way = rand()%good_ways;

    for(int i=0; i<4; i++)
    {
        if(ways[i]!=ACTION_VOID)
        {
            if(way==0)
                return ways[i];
            way--;
        }
    }

    return ACTION_DO_NOTHING;
}

static void clientb_behaviour(void)
{
    struct map_t map = clientc_get_map();

    int x=0;
    int y=0;
    clientc_get_pos(&x, &y);

    enum action_t direction;

    // Ucieczka przed bestią
    direction = indep_navigate_tile(&map, x, y, TILE_BEAST, 4);
    if(direction!=ACTION_VOID)
    {
        enum action_t escape_direction = clientb_escape(direction, &map, x, y);
        clientc_move(escape_direction);
        return;
    }

    // Powrót do obozu
    if(clientc_is_campside_known() && clientc_get_found_money()>MONEY_TO_RETURN)
    {
        direction = indep_navigate_tile(&map, x, y, TILE_CAMPSIDE, MAP_HEIGHT*MAP_WIDTH);
        if(direction!=ACTION_VOID)
        {
            clientc_move(direction);
            return;
        }
    }

    // Zbieraj dropy
    direction = indep_navigate_tile(&map, x, y, TILE_DROP, 4);
    if(direction!=ACTION_VOID)
    {
        clientc_move(direction);
        return;
    }

    // Zbieraj duże skarby
    direction = indep_navigate_tile(&map, x, y, TILE_L_TREASURE, 4);
    if(direction!=ACTION_VOID)
    {
        clientc_move(direction);
        return;
    }

    // Zbieraj małe skarby
    direction = indep_navigate_tile(&map, x, y, TILE_S_TREASURE, 4);
    if(direction!=ACTION_VOID)
    {
        clientc_move(direction);
        return;
    }

    // Zbieraj monety
    direction = indep_navigate_tile(&map, x, y, TILE_COIN, 4);
    if(direction!=ACTION_VOID)
    {
        clientc_move(direction);
        return;
    }

    // Odkrywaj
    direction = indep_navigate_tile(&map, x, y, TILE_UNKNOWN, MAP_WIDTH*MAP_HEIGHT);
    if(direction!=ACTION_VOID)
    {
        clientc_move(direction);
        return;
    }

    // Idz w wolne sąsiednie miejsce
    direction = indep_navigate_tile(&map, x, y, TILE_FLOOR, MAP_WIDTH*MAP_HEIGHT);
    if(direction!=ACTION_VOID)
    {
        clientc_move(direction);
        return;
    }

    // Nie rób nic
    clientc_move(ACTION_DO_NOTHING);
    return;   
}

static void *clientb_update_thread(void *ptr)
{
    while(1)
    {
        clientc_wait_for_data();
        clientc_update_client_data();
        clientc_display_stats();
        clientc_display_map();
        clientb_behaviour();
    }

    return NULL;
}

int main(void)
{
    current_action = ACTION_GO_UP;

    clientc_enter_server(CLIENT_TYPE_CPU);
    
    pthread_create(&update_thread, NULL, clientb_update_thread, NULL);
    pthread_create(&input_thread, NULL, clientb_input_thread, NULL);
    pthread_join(input_thread, NULL);

    clientc_leave_server();
    return 0;
}

