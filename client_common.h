#ifndef __CLIENT_COMMON_H__
#define __CLIENT_COMMON_H__

#include "common.h"

typedef enum action_t(*behaviour_fun_t)(void);

void clientc_run(enum client_type_t client_type);

#endif