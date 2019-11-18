#include <unistd.h>
#include <stdlib.h>
#include "server_data.h"
#include "common.h"
#include "map.h"

void sd_init(struct server_data_t *data)
{
    for(int i=0; i<MAX_CLIENTS_COUNT; i++)
    {
        struct server_client_data_t *client_data = data->clients_data + i;
        client_data->type = CLIENT_TYPE_FREE;
    }

    data->server_pid = getpid();
    data->round = 0;
    data->campside_x = 0;
    data->campside_y = 0;
}

void sd_add_client(struct server_data_t *data, int slot, int pid, enum client_type_t type)
{
    struct server_client_data_t *client_data = data->clients_data + slot;
    client_data->type = type;
    client_data->pid = pid;
    client_data->turns_to_wait = 0;
    sd_set_player_spawn(data, slot);
}

void sd_remove_client(struct server_data_t *data, int slot)
{
    struct server_client_data_t *client_data = data->clients_data + slot;
    client_data->type = CLIENT_TYPE_FREE;
}

void sd_move(struct server_data_t *sd, int slot, enum action_t action)
{
    struct server_client_data_t *client_data = sd->clients_data+slot;

    if(client_data->turns_to_wait>0)
    {
        client_data->turns_to_wait--;
        return;
    }

    if(action == ACTION_DO_NOTHING)
        return;

    int current_x = client_data->current_x;
    int current_y = client_data->current_y;

    int next_x = current_x;
    int next_y = current_y;

    if(action==ACTION_GO_DOWN) next_y++;
    else if(action==ACTION_GO_UP) next_y--;
    else if(action==ACTION_GO_LEFT) next_x--;
    else if(action==ACTION_GO_RIGHT) next_x++;

    enum tile_t dest_tile = map_get_tile(&sd->map, next_x, next_y);

    // Interakcja z podstawowymi elementami mapy
    if(dest_tile==TILE_WALL)
    {
        return;
    }
    if(dest_tile==TILE_BUSH)
    {
        client_data->turns_to_wait = 1;
    }
    else if(dest_tile==TILE_COIN)
    {
        client_data->coins_found += 1;
        sd->map.map[next_y][next_x] = TILE_FLOOR;
    }
    else if(dest_tile==TILE_S_TREASURE)
    {
        client_data->coins_found += SMALL_TREASURE_VALUE;
        sd->map.map[next_y][next_x] = TILE_FLOOR;
    }
    else if(dest_tile==TILE_L_TREASURE)
    {
        client_data->coins_found += BIG_TREASURE_VALUE;
        sd->map.map[next_y][next_x] = TILE_FLOOR;
    }
    else if(dest_tile==TILE_CAMPSIDE)
    {
        client_data->coins_brought += client_data->coins_found;
        client_data->coins_found = 0;
    }

    // Aktualizacja pozycji
    client_data->current_x = next_x;
    client_data->current_y = next_y;

    // Interakcja z obozami(Jeżli gracz jest w obozie to zderzenia z graczami nie obowiązują)
    if(client_data->current_x == sd->campside_x && client_data->current_y==sd->campside_y)
    {
        client_data->coins_brought += client_data->coins_found;
        client_data->coins_found = 0;
    }
    else
    {
        // Interakcja z innymi graczami
        int kill_player = 0;
        for(int i=0; i<MAX_CLIENTS_COUNT; i++)
        {
            struct server_client_data_t *client_data2 = sd->clients_data+i;
            if(client_data2->type!=CLIENT_TYPE_FREE)
            {
                if(i!=slot && client_data2->current_x==client_data->current_x && client_data2->current_y==client_data->current_y)
                {
                    kill_player = 1;
                    sd_player_kill(sd, i);
                }
            }
        }
        if(kill_player)
            sd_player_kill(sd, slot);
    }
    
    // Interakcja z dropami
    for(int i=0; i<(int)sd->dropped_data.size(); i++)
    {
        struct server_drop_data_t *drop = &(sd->dropped_data.at(i));
        if(client_data->current_x==drop->x && client_data->current_y==drop->y)
        {
            client_data->coins_found += drop->value;
            sd->dropped_data.erase(sd->dropped_data.begin()+i);
            i--;
        }
    }
}

void sd_fill_output_block(struct server_data_t *sd, int slot, struct map_t *complete_map, struct client_output_block_t *output)
{
    struct server_client_data_t *data = sd->clients_data+slot;

    output->x = data->current_x;
    output->y = data->current_y;

    output->coins_brought = data->coins_brought;
    output->coins_found = data->coins_found;

    output->deaths = data->deaths;

    output->round = sd->round;
    output->server_pid = sd->server_pid;

    sd_fill_surrounding_area(complete_map, data->current_x, data->current_y, &output->surrounding_area);
}

void sd_set_player_spawn(struct server_data_t *sd, int slot)
{
    struct server_client_data_t *client = sd->clients_data+slot;

    client->spawn_x = rand()%20;
    client->spawn_y = rand()%20;

    client->current_x = client->spawn_x;
    client->current_y = client->spawn_y;
}

void sd_generate_round(struct server_data_t *sd)
{
    sd->round++;

    for(int i=0; i<MAP_HEIGHT; i++)
    {
        for(int j=0; j<MAP_WIDTH; j++)
        {
            if(i==0 || j==0 || i==MAP_HEIGHT-1 || j==MAP_WIDTH-1)
                sd->map.map[i][j] = TILE_WALL;
            else
                sd->map.map[i][j] = TILE_FLOOR;
        }
    }

    sd->map.map[7][7] = TILE_COIN;
    sd->map.map[7][5] = TILE_S_TREASURE;
    sd->map.map[7][3] = TILE_L_TREASURE;

    sd->map.map[7][9] = TILE_BUSH;
    sd->map.map[6][9] = TILE_BUSH;

    sd->map.map[7][10] = TILE_WALL;
    sd->map.map[6][10] = TILE_WALL;

    sd->campside_x = 10;
    sd->campside_y = 10;

    map_generate_maze(&sd->map);

    for(int i=0; i<MAX_CLIENTS_COUNT; i++)
    {
        struct server_client_data_t *client = sd->clients_data+i;
        if(client->type!=CLIENT_TYPE_FREE)
        {
            sd_set_player_spawn(sd, i);
            sd->map.map[client->current_y][client->current_x] = (enum tile_t)(TILE_PLAYER1+i);
        }
    }
}

void sd_create_complete_map(struct server_data_t *sd, struct map_t *result_map)
{
    // Odbijanie tła mapy
    map_copy(&sd->map, result_map);

    // Odbijanie graczy na mapie
    for(int i=0; i<MAX_CLIENTS_COUNT; i++)
    {
        struct server_client_data_t *client = sd->clients_data+i;
        if(client->type!=CLIENT_TYPE_FREE)
            result_map->map[client->current_y][client->current_x] = (enum tile_t)(TILE_PLAYER1+i);
    }

    // Odbijanie dropu
    for(int i=0; i<(int)sd->dropped_data.size(); i++)
    {
        struct server_drop_data_t *drop = &(sd->dropped_data.at(i));
        result_map->map[drop->y][drop->x] = TILE_DROP;
    }

    // Odbicie obozowiska
    result_map->map[sd->campside_y][sd->campside_x] = TILE_CAMPSIDE;
}

void sd_player_kill(struct server_data_t *sd, int slot)
{
    struct server_client_data_t *client = sd->clients_data+slot;

    int drop_found = 0;
    for(int i=0; i<(int)sd->dropped_data.size(); i++)
    {
        struct server_drop_data_t *drop = &(sd->dropped_data.at(i));
        if(drop->x==client->current_x && drop->y==client->current_y)
        {
            drop->value += client->coins_found;
            drop_found = 1;
            break;
        }
    }

    if(!drop_found)
    {
        struct server_drop_data_t new_drop = { client->current_x, client->current_y, client->coins_found };
        sd->dropped_data.push_back(new_drop);
    }

    client->deaths++;
    client->current_x = client->spawn_x;
    client->current_y = client->spawn_y;
    client->coins_found = 0;
}

void sd_fill_surrounding_area(struct map_t *complete_map, int cx, int cy, surrounding_area_t *area)
{
    for(int i=0; i<VISIBLE_AREA_SIZE; i++)
    {
        for(int j=0; j<VISIBLE_AREA_SIZE; j++)
        {
            int rel_y = i-VISIBLE_DISTANCE;
            int rel_x = j-VISIBLE_DISTANCE;

            enum tile_t tile = map_get_tile(complete_map, cx+rel_x, cy+rel_y);
            (*area)[i][j] = tile;
        }
    }
}