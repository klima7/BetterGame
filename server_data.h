#ifndef __SERVER_DATA_H__
#define __SERVER_DATA_H__

#include <stdint.h>
#include <vector>
#include "server_data.h"
#include "common.h"
#include "map.h"

struct server_client_data_t
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

    int turns_to_wait;
};

struct server_drop_data_t
{
    int x;
    int y;
    int value;
};

struct server_data_t
{
    int server_pid;
    int round;

    struct map_t map;

    int campside_x;
    int campside_y;

    std::vector<struct server_drop_data_t> dropped_data;
    struct server_client_data_t clients_data[MAX_CLIENTS_COUNT];
};

void sd_init(struct server_data_t *data);
void sd_add_client(struct server_data_t *data, int slot, int pid, enum client_type_t type);
void sd_remove_client(struct server_data_t *data, int slot);
void sd_move(struct server_data_t *data, int slot, enum action_t action);
void sd_fill_output_block(struct server_data_t *sd, int slot, struct map_t *complete_map, struct client_output_block_t *output);
void sd_set_player_spawn(struct server_data_t *sd, int slot);
void sd_generate_round(struct server_data_t *sd);
void sd_create_complete_map(struct server_data_t *sd, struct map_t *result_map);
void sd_player_kill(struct server_data_t *sd, int slot);
void sd_fill_surrounding_area(struct map_t *complete_map, int cx, int cy, surrounding_area_t *area);

#endif