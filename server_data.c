#include <unistd.h>
#include <stdlib.h>
#include "server_data.h"
#include "common.h"

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
    sd_set_player_spawn(data, slot);
    data->map.map[client_data->current_y][client_data->current_x] = TILE_PLAYER1 + slot;
}

void sd_remove_client(struct server_data_t *data, int slot)
{
    struct server_client_data_t *client_data = data->clients_data + slot;
    client_data->type = CLIENT_TYPE_FREE;
    data->map.map[client_data->current_y][client_data->current_x] = TILE_FLOOR;
}

void sd_move(struct server_data_t *data, int slot, enum action_t action)
{
    struct server_client_data_t *client_data = data->clients_data+slot;

    if(action==ACTION_GO_DOWN)
    {
        client_data->current_y++;
    }

    else if(action==ACTION_GO_UP)
    {
        client_data->current_y--;
    }

    else if(action==ACTION_GO_LEFT)
    {
        client_data->current_x--;
    }

    else if(action==ACTION_GO_RIGHT)
    {
        client_data->current_x++;
    }
}

void sd_fill_output_block(struct server_data_t *sd, struct client_output_block_t *output, int slot)
{
    struct server_client_data_t *data = sd->clients_data+slot;

    output->x = data->current_x;
    output->y = data->current_y;

    output->coins_brought = data->coins_brought;
    output->coins_found = data->coins_found;

    output->deaths = data->deaths;

    output->round = sd->round;
    output->server_pid = sd->server_pid;
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

    sd->map.map[10][10] = TILE_CAMPSICE;

    sd->campside_x = 10;
    sd->campside_y = 10;

    for(int i=0; i<MAX_CLIENTS_COUNT; i++)
    {
        struct server_client_data_t *client = sd->clients_data+i;
        if(client->type!=CLIENT_TYPE_FREE)
        {
            sd_set_player_spawn(sd, i);
            sd->map.map[client->current_y][client->current_x] = TILE_PLAYER1 + i;
        }
    }
}