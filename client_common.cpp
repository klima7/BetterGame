#include <stdio.h>
#include <stdint.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <ncursesw/ncurses.h>
#include <fcntl.h>
#include <unistd.h>
#include <locale.h>
#include <time.h>
#include <stdlib.h>
#include "client_common.h"
#include "client_data.h"
#include "common.h"
#include "map.h"
#include "tiles.h"

// Funkcje statyczne
static void clientc_init_ncurses(void);
void clientc_shift_if_too_far(void);
static int cclient_enter_free_server_slot(enum client_type_t client_type);

// Pamięć współdzielona
int fd;
struct clients_sm_block_t *sm_block;
struct client_sm_block_t *my_sm_block;

// Dane klienta - oddzielenie mechanizmu komunikacji od danych
struct client_data_t client_data;

// Wszystkie wyświetlane okna
WINDOW *stat_window;
WINDOW *map_window;
WINDOW *help_window;

// Inicjacja ncurses i okien
static void clientc_init_ncurses(void)
{
    setlocale(LC_ALL, "");
	initscr();
  	noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    cbreak();

    init_colors();

    stat_window = newwin(12, 30, 0, 0);
    map_window = newwin(MAP_VIEW_HEIGHT+2, MAP_VIEW_WIDTH+4, 4, 40);
    help_window = newwin(18, 30, 12, 0);

    bkgd(COLOR_PAIR(COLOR_BLACK_ON_WHITE));
    refresh();
    wbkgdset(stat_window, COLOR_PAIR(COLOR_BLACK_ON_WHITE));
    wbkgdset(map_window, COLOR_PAIR(COLOR_BLACK_ON_WHITE));
    wbkgdset(help_window, COLOR_PAIR(COLOR_BLACK_ON_WHITE));
}

// Znalezienie wolnego miejsca na serwerze i zabranie go
static int cclient_enter_free_server_slot(enum client_type_t client_type)
{
    int slot = -1;

    for(int i=0; i<MAX_CLIENTS_COUNT; i++)
    {
        struct client_sm_block_t *client_block = sm_block->clients+i;
        enter_cs(&client_block->data_cs);
        if(client_block->data_block.client_type==CLIENT_TYPE_FREE)
        {
            slot = i;
            client_block->data_block.client_type = client_type;
            client_block->data_block.client_pid = getpid();
            client_block->input_block.action = ACTION_DO_NOTHING;
            client_block->input_block.respond_flag = 1;
            exit_cs(&client_block->data_cs);
            break;
        }
        exit_cs(&client_block->data_cs);
    }

    return slot;
}

// Dołączenie klienta na serwer
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
}

// Wyświetlenie danych gracza
void clientc_display_stats(void)
{
    werase(stat_window);

    int line = 0;
    wattron(stat_window, COLOR_PAIR(COLOR_WHITE_ON_RED));
    mvwprintw(stat_window, line++, 0, "---Server Information---");
    wattron(stat_window, COLOR_PAIR(COLOR_BLACK_ON_WHITE));
    mvwprintw(stat_window, line++, 0, "Servers pid  : %d", client_data.server_pid);


    if(client_data.visible_map.campside_x==-1 && client_data.visible_map.campside_y==-1)
        mvwprintw(stat_window, line++, 0, "Campside X/Y : unknown");
    else
        mvwprintw(stat_window, line++, 0, "Campside X/Y : %d/%d", client_data.visible_map.campside_x, client_data.visible_map.campside_y);
    mvwprintw(stat_window, line++, 0, "Round        : %d", client_data.round_number);

    line++;

    wattron(stat_window, COLOR_PAIR(COLOR_WHITE_ON_RED));
    mvwprintw(stat_window, line++, 0, "---Client Information---");
    wattron(stat_window, COLOR_PAIR(COLOR_BLACK_ON_WHITE));

    const char *message = NULL;
    if(client_data.type==CLIENT_TYPE_HUMAN) message="HUMAN";
    else if(client_data.type==CLIENT_TYPE_CPU) message="CPU";

    mvwprintw(stat_window, line++, 0, "Number       : %d", client_data.slot+1);
    mvwprintw(stat_window, line++, 0, "Type         : %s", message);

    mvwprintw(stat_window, line++, 0, "Pos X/Y      : %d/%d", client_data.current_x, client_data.current_y);
    mvwprintw(stat_window, line++, 0, "Deaths       : %d", client_data.deaths);
    mvwprintw(stat_window, line++, 0, "Coins        : %d/%d", client_data.coins_found, client_data.coins_brought);

    wrefresh(stat_window);
}

// Czeka na dane od serwera i aktualizuje własne dane
void clientc_wait_and_update(void)
{
    // Wait - czeka na dane od serwera, ograniczone czasowo
    struct timespec tolerated_time;
    clock_gettime(CLOCK_REALTIME, &tolerated_time);
    tolerated_time.tv_sec += (TURN_TIME+DATA_WAITING_TIME_MAX) / 1000000;
    long temp_ns = tolerated_time.tv_nsec + (TURN_TIME+DATA_WAITING_TIME_MAX)%1000000*1000;
    tolerated_time.tv_sec += temp_ns / 1000000000;
    tolerated_time.tv_nsec = temp_ns % 1000000000;

    int res = sem_timedwait(&my_sm_block->output_block_sem, &tolerated_time);
    if(res!=0)
    {
        display_center("Server doesn't respond, exiting in 3 seconds");
        usleep(3e6);
        munmap(sm_block, SHARED_BLOCK_SIZE);
        close(fd);
        endwin();
        delwin(stat_window);
        delwin(map_window);
        exit(0);
    }

    // Update - Aktualizuje dane klienta za pomocą otrzymanych danych
    enter_cs(&my_sm_block->data_cs);
    my_sm_block->input_block.respond_flag = 1;
    cd_update_with_output_block(&client_data, &my_sm_block->output_block);
    exit_cs(&my_sm_block->data_cs);
}

// Automatycznie scroluje mapę jeżeli gracz jest zbyt blisko krańca
void clientc_shift_if_too_far(void)
{
    int x_on_map = client_data.current_x-client_data.visible_map.viewpoint_x;
    int y_on_map = client_data.current_y-client_data.visible_map.viewpoint_y;

    if(y_on_map<SHIFT_MARGIN_Y)
        map_shift(&client_data.visible_map, 0, y_on_map-SHIFT_MARGIN_Y); 
    else if(y_on_map>=MAP_VIEW_HEIGHT-SHIFT_MARGIN_Y)
        map_shift(&client_data.visible_map, 0, y_on_map-MAP_VIEW_HEIGHT+SHIFT_MARGIN_Y+1); 

    if(x_on_map<SHIFT_MARGIN_X)
        map_shift(&client_data.visible_map, x_on_map-SHIFT_MARGIN_X, 0); 
    else if(x_on_map>=MAP_VIEW_WIDTH-SHIFT_MARGIN_X)
        map_shift(&client_data.visible_map, x_on_map-MAP_VIEW_WIDTH+SHIFT_MARGIN_X+1, 0); 
}

// Wyświetla mapę
void clientc_display_map(void)
{
    clientc_shift_if_too_far();
    map_display(&client_data.visible_map, map_window);
}

// Opuszczenie serwera
void clientc_leave_server(void)
{
    enter_cs(&my_sm_block->data_cs);
    my_sm_block->data_block.client_type = CLIENT_TYPE_FREE;
    exit_cs(&my_sm_block->data_cs);
    munmap(sm_block, SHARED_BLOCK_SIZE);
    close(fd);
    endwin();
    delwin(stat_window);
    delwin(map_window);
    delwin(help_window);
}

// Ruch gracza
void clientc_move(enum action_t action)
{
    enter_cs(&my_sm_block->data_cs);
    my_sm_block->input_block.action = action;
    exit_cs(&my_sm_block->data_cs);
}

// Wyświetla cały interfejs klienta
void clientc_display(void)
{
    clientc_display_stats();
    clientc_display_map();
    display_help_window(help_window);
}



// Pobranie mapy znanej przez gracza 
struct map_t clientc_get_map(void)
{
    return client_data.visible_map;
}

// Pobranie liczby monet pozbieranych przez gracza
int clientc_get_found_money(void)
{
    return client_data.coins_found;
}

// Pobranie aktualnej pozycji gracza
void clientc_get_pos(int *x, int *y)
{
    *x = client_data.current_x;
    *y = client_data.current_y;
}

// Czy pozycja obozowiska jest znana 
int clientc_is_campside_known(void)
{
    if(client_data.visible_map.campside_x==-1 && client_data.visible_map.campside_y==-1)
        return 0;
    else 
        return 1;
}

