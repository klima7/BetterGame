#ifndef __SERVER_DATA_H__
#define __SERVER_DATA_H__

#include <stdint.h>
#include "server_data.h"
#include "common.h"

struct client_data_t
{
    int pid;
    enum client_type_t type;

    int spawn_x;
    int spawn_y;

    int current_x;
    int current_y;

    int coins_found;
    int coins_brought;

    int deaths;
};

struct server_data_t
{
    int round;

    int campside_x;
    int campside_y;

    struct client_data_t clients_data[MAX_CLIENTS_COUNT];

    uint8_t game_map[MAP_HEIGHT][MAP_WIDTH];
};

void sd_init(struct server_data_t *data);
void sd_add_client(struct server_data_t *data, int slot, int pid, enum client_type_t type);
void sd_remove_client(struct server_data_t *data, int slot);

#endif