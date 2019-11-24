#ifndef __BEAST_H__
#define __BEAST_H__

#include <pthread.h>
#include "common.h"

struct beast_t
{
    int x;
    int y;
    int turns_to_wait;

    action_t current_direction;
    int turns_to_stay;
};

void beast_init(struct beast_t *beast, int x, int y);
void beast_update(struct server_data_t *sd, int nr);
int beast_see_player(struct beast_t *beast, struct map_t *map);

#endif