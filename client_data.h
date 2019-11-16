#ifndef __CLIENT_DATA_H__
#define __CLIENT_DATA_H__

#include "common.h"

enum campside_status_t { CAMPSIDE_KNOWN, CAMPWIDE_UNKNOWN };

struct client_data_t
{
    int server_pid;
    int my_pid;

    enum campside_status_t campside_status;
    int campside_x;
    int campside_y;

    enum client_type_t type;
    int round_number;

    int current_x;
    int current_y;

    int coins_found;
    int coins_brought;

    int deaths;
};

void cd_init(struct client_data_t* cd, enum client_type_t type);
void cd_update_with_output_block(struct client_data_t* cd, struct client_output_block_t *output);

#endif