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
#include "tiles.h"

// Ile monet musi zebrać bot aby postanowić wrócić do bazy
#define MONEY_TO_RETURN 100

// Funkcje statyczne
static void *clientb_input_thread(void *ptr);
static void *clientb_update_thread(void *ptr);
static void clientb_behaviour(void);
static action_t clientb_escape(enum action_t beast_dir, struct map_t *map, int x, int y);

// Wątki
pthread_t input_thread;
pthread_t update_thread;

// Kierunek w którym bot aktualnie się przemieszcza
enum action_t current_direction;

// Poprzednia pozycja, służy do sprawdzenia, czy botowi udało się ruszyć - może na przykład być w krzakach i trzeba potwórzyć ostatni ruch
int last_x;
int last_y;

// Wątek obsługujący klawiature
static void *clientb_input_thread(void *ptr)
{
    while(1)
    {
        int c = getch();
        if(tolower(c)=='q')
            return NULL;
    }
}

// W którą stronę powinien uciec klient przed bestią znajdującą się w kierunku beast_dir
static action_t clientb_escape(enum action_t beast_dir, struct map_t *map, int x, int y)
{
    enum action_t ways[] = { ACTION_GO_UP, ACTION_GO_DOWN, ACTION_GO_LEFT, ACTION_GO_RIGHT };
    
    for(int i=0; i<4; i++)
    {
        if(ways[i]==beast_dir)
            ways[i] = ACTION_VOID;
    }

    if(!tile_is_walkable(map_get_tile(map, x, y-1))) ways[0] = ACTION_VOID;
    if(!tile_is_walkable(map_get_tile(map, x, y+1))) ways[1] = ACTION_VOID;
    if(!tile_is_walkable(map_get_tile(map, x-1, y))) ways[2] = ACTION_VOID;
    if(!tile_is_walkable(map_get_tile(map, x+1, y))) ways[3] = ACTION_VOID;

    int good_ways = 0;
    for(int i=0; i<4; i++)
    {
        if(ways[i]!=ACTION_VOID)
            good_ways++;
    }

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

// Zachowanie bota
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
        current_direction = escape_direction;
        return;
    }

    // Powrót do obozu
    if(clientc_is_campside_known() && clientc_get_found_money()>MONEY_TO_RETURN)
    {
        direction = indep_navigate_tile(&map, x, y, TILE_CAMPSIDE, MAP_HEIGHT);
        if(direction!=ACTION_VOID)
        {
            clientc_move(direction);
            current_direction = direction;
            return;
        }
    }

    // Zbieraj dropy
    direction = indep_navigate_tile(&map, x, y, TILE_DROP, 4);
    if(direction!=ACTION_VOID)
    {
        clientc_move(direction);
        current_direction = direction;
        return;
    }

    // Zbieraj duże skarby
    direction = indep_navigate_tile(&map, x, y, TILE_L_TREASURE, 4);
    if(direction!=ACTION_VOID)
    {
        clientc_move(direction);
        current_direction = direction;
        return;
    }

    // Zbieraj małe skarby
    direction = indep_navigate_tile(&map, x, y, TILE_S_TREASURE, 4);
    if(direction!=ACTION_VOID)
    {
        clientc_move(direction);
        current_direction = direction;
        return;
    }

    // Zbieraj monety
    direction = indep_navigate_tile(&map, x, y, TILE_COIN, 4);
    if(direction!=ACTION_VOID)
    {
        clientc_move(direction);
        current_direction = direction;
        return;
    }

    // Podąża lewą ścianą
    direction = indep_follow_left_wall(&map, x, y, current_direction);
    if(x != last_x || y != last_y)
        current_direction = direction;
    clientc_move(current_direction);
    last_x = x;
    last_y = y;
    return;
}

// Wątek aktualizujący
static void *clientb_update_thread(void *ptr)
{
    while(1)
    {
        clientc_wait_and_update();
        clientb_behaviour();
        clientc_display();
    }
    return NULL;
}

// Funkcja main
int main(void)
{
    current_direction = ACTION_DO_NOTHING;

    // Dołączenie na serwer
    clientc_enter_server(CLIENT_TYPE_CPU);
    
    // Stworzenie wątków
    pthread_create(&update_thread, NULL, clientb_update_thread, NULL);
    pthread_create(&input_thread, NULL, clientb_input_thread, NULL);
    pthread_join(input_thread, NULL);

    // Opuszczenie serwera
    clientc_leave_server();
    return 0;
}

