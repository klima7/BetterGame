#include "server_data.h"
#include "common.h"

void sd_init(struct server_data_t *data)
{
    for(int i=0; i<MAX_CLIENTS_COUNT; i++)
    {
        struct client_data_t *client_data = data->clients_data + i;
        client_data->type = CLIENT_TYPE_FREE;
    }

    data->round = 0;
    data->campside_x = 0;
    data->campside_y = 0;
}

void sd_add_client(struct server_data_t *data, int slot, int pid, enum client_type_t type)
{
    struct client_data_t *client_data = data->clients_data + slot;
    client_data->type = type;
    client_data->pid = pid;
}

void sd_remove_client(struct server_data_t *data, int slot)
{
    struct client_data_t *client_data = data->clients_data + slot;
    client_data->type = CLIENT_TYPE_FREE;
}