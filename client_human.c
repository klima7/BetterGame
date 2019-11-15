#include <stdio.h>
#include <pthread.h>
#include "client_common.h"
#include "common.h"

pthread_t client_thread;

int main(void)
{
    clientc_enter_server(CLIENT_TYPE_HUMAN);
    getchar();
    clientc_leave_server();
    return 0;
}

