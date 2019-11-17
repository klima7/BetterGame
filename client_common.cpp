#include <stdio.h>
#include <stdint.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <ncursesw/ncurses.h>
#include <fcntl.h>
#include <unistd.h>
#include <locale.h>
#include "client_common.h"
#include "client_data.h"
#include "common.h"

// Funkcje statyczne
static void clientc_init_ncurses(void);
static int cclient_enter_free_server_slot(enum client_type_t client_type);

int fd;
struct clients_sm_block_t *sm_block;
struct client_sm_block_t *my_sm_block;

struct client_data_t client_data;

WINDOW *stat_window;

static void clientc_init_ncurses(void)
{
    setlocale(LC_ALL, "");
	initscr();
  	noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    cbreak();

    init_colors();

    stat_window = newwin(32, 50, 0, 0);
}

static int cclient_enter_free_server_slot(enum client_type_t client_type)
{
    int slot = -1;

    for(int i=0; i<MAX_CLIENTS_COUNT; i++)
    {
        struct client_sm_block_t *client_block = sm_block->clients+i;
        sem_wait(&client_block->data_cs);
        if(client_block->data_block.client_type==CLIENT_TYPE_FREE)
        {
            slot = i;
            client_block->data_block.client_type = client_type;
            client_block->data_block.client_pid = client_data.my_pid;
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

    fd = shm_open(SHM_FILE_NAME, O_RDWR, 0600);
    check(fd!=-1, "Server is probably not running, start server first");

    sm_block = (struct clients_sm_block_t *)mmap(NULL, SHARED_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    check(sm_block!=MAP_FAILED, "mmap error, press any key to quit");

    int occupied_slot = cclient_enter_free_server_slot(client_type);
    check(occupied_slot!=-1, "Server is full, you are not able to join");
    my_sm_block = sm_block->clients+occupied_slot;

    cd_init(&client_data, client_type, occupied_slot);

    display_center("Wait...");
}

void clientc_display_stats(void)
{
    wclear(stat_window);

    int line = 0;
    mvwprintw(stat_window, line++, 0, "Servers pid  : %d", client_data.server_pid);

    if(client_data.campside_status==CAMPWIDE_UNKNOWN)
        mvwprintw(stat_window, line++, 0, "Campside X/Y : unknown");
    else
        mvwprintw(stat_window, line++, 0, "Campside X/Y : %d/%d", client_data.campside_x, client_data.campside_y);
    mvwprintw(stat_window, line++, 0, "Round        : %d", client_data.round_number);

    line++;

    const char *message = NULL;
    if(client_data.type==CLIENT_TYPE_HUMAN) message="HUMAN";
    else if(client_data.type==CLIENT_TYPE_CPU) message="CPU";

    mvwprintw(stat_window, line++, 0, "Type         : %s", message);

    mvwprintw(stat_window, line++, 0, "Pos X/Y      : %d/%d", client_data.current_x, client_data.current_y);
    mvwprintw(stat_window, line++, 0, "Deaths       : %d", client_data.deaths);
    mvwprintw(stat_window, line++, 0, "Coins        : %d/%d", client_data.coins_found, client_data.coins_brought);

    wrefresh(stat_window);
}

void clientc_leave_server(void)
{
    sem_wait(&my_sm_block->data_cs);
    my_sm_block->data_block.client_type = CLIENT_TYPE_FREE;
    sem_post(&my_sm_block->data_cs);
    munmap(sm_block, SHARED_BLOCK_SIZE);
    close(fd);
    endwin();
    delwin(stat_window);
}

void clientc_move(enum action_t action)
{
    sem_wait(&my_sm_block->data_cs);
    my_sm_block->input_block.action = action;
    sem_post(&my_sm_block->data_cs);
}

