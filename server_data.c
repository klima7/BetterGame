#include <unistd.h>
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
}

void sd_remove_client(struct server_data_t *data, int slot)
{
    struct server_client_data_t *client_data = data->clients_data + slot;
    client_data->type = CLIENT_TYPE_FREE;
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