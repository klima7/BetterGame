#ifndef __CLIENT_COMMON_H__
#define __CLIENT_COMMON_H__

#include "common.h"

void clientc_enter_server(enum client_type_t client_type);
void clientc_leave_server(void);
void clientc_move(enum action_t action);
void clientc_display_stats(void);
void clientc_display_map(void);
void clientc_wait_for_data(void);
void clientc_update_client_data(void);

#endif