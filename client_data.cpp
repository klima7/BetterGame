#include <unistd.h>
#include "client_data.h"
#include "common.h"
#include "map.h"

void cd_init(struct client_data_t* cd, enum client_type_t type, int slot)
{
    cd->my_pid = getpid();
    cd->type = type;
    cd->slot = slot;
    map_fill(&cd->visible_map, TILE_UNKNOWN);
    cd->visible_map.campside_x = -1;
    cd->visible_map.campside_y = -1;
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

    map_remove_unsure_tiles(&cd->visible_map);
    map_update_with_surrounding_area(&cd->visible_map, &output->surrounding_area, output->x, output->y);
}