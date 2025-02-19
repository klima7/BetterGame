#ifndef __CLIENT_DATA_H__
#define __CLIENT_DATA_H__

#include "common.h"
#include "map.h"

// Dane klienta po stronie klienta
struct client_data_t
{
    int server_pid;
    int my_pid;
    int slot;

    enum client_type_t type;
    int round_number;

    int current_x;
    int current_y;

    int coins_found;
    int coins_brought;

    int deaths;
    struct map_t visible_map;
};

// Prototypy
void cd_init(struct client_data_t* cd, enum client_type_t type, int slot);
void cd_update_with_output_block(struct client_data_t* cd, struct client_output_block_t *output);

#endif