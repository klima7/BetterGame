#ifndef __CLIENT_COMMON_H__
#define __CLIENT_COMMON_H__

#include "common.h"
#include "map.h"

// Kiedy mapa ma być przesówana - odległość od krańców
#define SHIFT_MARGIN_X 4
#define SHIFT_MARGIN_Y 4

// Prototypy
void clientc_enter_server(enum client_type_t client_type);
void clientc_leave_server(void);
void clientc_move(enum action_t action);
void clientc_display_stats(void);
void clientc_display_map(void);
void clientc_display(void);
void clientc_wait_and_update(void);
struct map_t clientc_get_map(void);
int clientc_get_found_money(void);
void clientc_get_pos(int *x, int *y);
int clientc_is_campside_known(void);

#endif