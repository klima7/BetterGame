#ifndef __CLIENT_COMMON_H__
#define __CLIENT_COMMON_H__

#include "common.h"

typedef enum action_t(*behaviour_fun_t)(void);

void clientc_enter_server(enum client_type_t client_type);
void clientc_leave_server(void);

#endif