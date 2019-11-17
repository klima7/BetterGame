#include <stdio.h>
#include <pthread.h>
#include <ncursesw/ncurses.h>
#include <ctype.h>
#include "client_common.h"
#include "common.h"
#include "client_data.h"

pthread_t input_thread;
pthread_t update_thread;

// Funkcje statyczne
static void *clienth_input_thread(void *ptr);
static void *clienth_update_thread(void *ptr);

static void *clienth_input_thread(void *ptr)
{
    while(1)
    {
        int c = getch();
        if(c==KEY_LEFT)
            clientc_move(ACTION_GO_LEFT);
        else if(c==KEY_RIGHT)
            clientc_move(ACTION_GO_RIGHT);
        else if(c==KEY_UP)
            clientc_move(ACTION_GO_UP);
        else if(c==KEY_DOWN)
            clientc_move(ACTION_GO_DOWN);
        else if(tolower(c)=='q')
            return NULL;
    }
}

static void *clienth_update_thread(void *ptr)
{
    while(1)
    {
        clientc_wait_for_data();
        clientc_update_client_data();
        clientc_display_stats();
        clientc_display_map();
    }

    return NULL;
}

int main(void)
{
    clientc_enter_server(CLIENT_TYPE_HUMAN);
    
    pthread_create(&update_thread, NULL, clienth_update_thread, NULL);
    pthread_create(&input_thread, NULL, clienth_input_thread, NULL);
    pthread_join(input_thread, NULL);

    clientc_leave_server();
    return 0;
}

