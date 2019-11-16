#include <unistd.h>
#include "client_data.h"
#include "common.h"

void cd_init(struct client_data_t* cd, enum client_type_t type)
{
    cd->my_pid = getpid();
    cd->campside_status = CAMPWIDE_UNKNOWN;
    cd->type = type;
}

void cd_update_with_output_block(struct client_data_t* cd, struct client_output_block_t *output)
{
    cd->server_pid = output->server_pid;
    cd->round_number = output->round;
    cd->current_x = output->x;
    cd->current_y = output->y;
    cd->coins_found = output->coins_found;
    cd->coins_brought = output->coins_brought;
    cd->deaths = output->deaths;
}