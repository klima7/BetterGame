#include <stdio.h>
#include <stdint.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <ncursesw/ncurses.h>
#include <fcntl.h>
#include <unistd.h>
#include "client_common.h"
#include "common.h"

// Funkcje statyczne
static void clientc_init_ncurses(void);
static int cclient_enter_free_server_slot(enum client_type_t client_type);

int fd;
struct clients_sm_block_t *sm_block;

int my_pid;
struct client_sm_block_t *my_sm_block;

static void clientc_init_ncurses(void)
{
	initscr();
  	noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    cbreak();
}

static int cclient_enter_free_server_slot(enum client_type_t client_type)
{
    int slot = -1;

    for(int i=0; i<MAX_CLIENTS_COUNT; i++)
    {
        client_sm_block_t *client_block = sm_block->clients+i;
        sem_wait(&client_block->data_cs);
        if(client_block->data_block.client_type==CLIENT_TYPE_FREE)
        {
            slot = i;
            client_block->data_block.client_type = client_type;
            client_block->data_block.client_pid = my_pid;
            client_block->input_block.action = ACTION_DO_NOTHING;
            sem_post(&client_block->data_cs);
            break;
        }
        sem_post(&client_block->data_cs);
    }

    return slot;
}

void clientc_enter_server(enum client_type_t client_type)
{
    clientc_init_ncurses();
    my_pid = getpid();

    fd = shm_open(SHM_FILE_NAME, O_RDWR, 0600);
    check(fd!=-1, "Server is probably not running, start server first");

    sm_block = (struct clients_sm_block_t *)mmap(NULL, SHARED_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    check(sm_block!=MAP_FAILED, "mmap error, press any key to quit");

    int occupied_slot = cclient_enter_free_server_slot(client_type);
    check(occupied_slot!=-1, "Server is full, you are not able to join");
    my_sm_block = sm_block->clients+occupied_slot;
}

void clientc_leave_server(void)
{
    sem_wait(&my_sm_block->data_cs);
    my_sm_block->data_block.client_type = CLIENT_TYPE_FREE;
    sem_post(&my_sm_block->data_cs);
    munmap(sm_block, SHARED_BLOCK_SIZE);
    close(fd);
    endwin();
}

