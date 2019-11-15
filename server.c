#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <ncursesw/ncurses.h>
#include "common.h"
#include "server_data.h"

#define LOG_LINES_COUNT 5
#define LOG_LINE_WIDTH 30
#define TURN_TIME 1000000

// Makro
#define SERVER_ADD_LOG(__msg, args...)\
for(int i=LOG_LINES_COUNT-1; i>0; i--)\
strncpy(logs[i], logs[i-1], LOG_LINE_WIDTH);\
snprintf(logs[0], LOG_LINE_WIDTH, __msg, ## args);

// Prototypy
void *server_display_thread(void *ptr);
void *server_input_thread(void *ptr);
void server_init_ncurses(void);
void server_init_sm(void);
void server_display_stats(void);
void server_display_logs(void);
void *server_update_thread(void *ptr);

int fd;
struct clients_sm_block_t *sm_block;
int server_pid;

WINDOW *stat_window;
WINDOW *log_window;

char logs[LOG_LINES_COUNT][LOG_LINE_WIDTH+1];

pthread_t display_thread;
pthread_t input_thread;
pthread_t update_thread;

struct server_data_t server_data;

void *server_display_thread(void *ptr)
{
    while(1)
    {
        server_display_stats();
        server_display_logs();
        usleep(100000);
    }
}

void *server_input_thread(void *ptr)
{
    while(1)
    {
        int c = getchar();
        if(tolower(c)=='q') 
        {
            SERVER_ADD_LOG("Confirm exiting with y/n");
            c = getchar();
            if(c=='y') return NULL;
            else SERVER_ADD_LOG("Exiting canceled");
        }
        else if(tolower(c)=='b')
        {
            SERVER_ADD_LOG("Adding beast");
        }
        else if(c=='c')
        {
            SERVER_ADD_LOG("Adding coint");
        }
        else if(c=='t')
        {
            SERVER_ADD_LOG("Adding small treasure");
        }
        else if(c=='T')
        {
            SERVER_ADD_LOG("Adding big treasure");
        }
    }
}

void *server_update_thread(void *ptr)
{
    while(1)
    {
        for(int i=0; i<MAX_CLIENTS_COUNT; i++)
        {
            client_sm_block_t *client_block = sm_block->clients+i;
            sem_wait(&client_block->data_cs);
            client_type_t type = client_block->data_block.client_type;

            if(type==CLIENT_TYPE_FREE)
            {
                if(server_data.clients_data[i].type != CLIENT_TYPE_FREE)
                {
                    sd_remove_client(&server_data, i);
                    SERVER_ADD_LOG("Client exited");
                }
            }

            else
            {
                //action_t action = client_block->input_block.action;
                client_block->input_block.action = ACTION_DO_NOTHING;

                int pid = client_block->data_block.client_pid;
                if(server_data.clients_data[i].type == CLIENT_TYPE_FREE || server_data.clients_data[i].pid != pid)
                {
                    sd_add_client(&server_data, i, pid, type);
                    SERVER_ADD_LOG("New client joinded");
                }

            }

            sem_post(&client_block->data_cs);
        }

        usleep(TURN_TIME);
    }
}

void server_init_ncurses(void)
{
	initscr();
  	noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    cbreak();

    stat_window = newwin(32, 50, 0, 0);
    log_window = newwin(LOG_LINES_COUNT+1, LOG_LINE_WIDTH, 34, 0);
}

void server_init_sm(void)
{
    fd = shm_open(SHM_FILE_NAME, O_CREAT | O_RDWR, 0600);
    check(fd!=-1, "shm_open error");

    int res = ftruncate(fd, SHARED_BLOCK_SIZE);
    check(res!=-1, "ftruncate error");

    sm_block = (struct clients_sm_block_t *)mmap(NULL, SHARED_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    check(sm_block!=MAP_FAILED, "mmap error");

    for(int i=0; i<MAX_CLIENTS_COUNT; i++)
    {
        client_sm_block_t *client_block = sm_block->clients+i;

        client_block->data_block.client_pid = 0;
        client_block->data_block.client_type = CLIENT_TYPE_FREE;
        client_block->input_block.action = ACTION_DO_NOTHING;

        sem_init(&client_block->data_cs, 1, 1);
        sem_init(&client_block->output_block_sem, 1, 0);
    }
}

void server_display_stats(void)
{
    wclear(stat_window);

    mvwprintw(stat_window, 0, 0, "Servers PID  : %d", server_pid);
    mvwprintw(stat_window, 1, 0, "Campside X/Y : %d/%d", server_data.campside_x, server_data.campside_y);
    mvwprintw(stat_window, 2, 0, "Round Number : %d", server_data.round);

    int line = 5;

    for(int i=0; i<MAX_CLIENTS_COUNT; i++)
    {
        enum client_type_t type = server_data.clients_data[i].type;

        mvwprintw(stat_window, line++, 0, "--PLAYER %d--", i+1);

        const char *message = NULL;
        if(type==CLIENT_TYPE_FREE) message="----";
        else if(type==CLIENT_TYPE_HUMAN) message="HUMAN";
        else if(type==CLIENT_TYPE_CPU) message="CPU";
        mvwprintw(stat_window, line++, 0, "Type:   %s", message);

        if(type==CLIENT_TYPE_FREE)
        {
            mvwprintw(stat_window, line++, 0, "PID:    ----");
            mvwprintw(stat_window, line++, 0, "Pos:    ----");
            mvwprintw(stat_window, line++, 0, "Deaths: ----");
            mvwprintw(stat_window, line++, 0, "Coins:  ----");
            line++;
        }

        else
        {
            struct client_data_t *client_data = server_data.clients_data+i;

            mvwprintw(stat_window, line++, 0, "PID:    %d", client_data->pid);
            mvwprintw(stat_window, line++, 0, "Pos:    %d/%d", client_data->current_x, client_data->current_y);
            mvwprintw(stat_window, line++, 0, "Deaths: %d", client_data->deaths);
            mvwprintw(stat_window, line++, 0, "Coins:  %d/%d", client_data->coins_found, client_data->coins_brought);
            line++;
        }
    }
    wrefresh(stat_window);
}

void server_display_logs(void)
{
    wclear(log_window);
    int line=0;
    mvwprintw(log_window, line++, 0, "--------Logs--------");
    for(int i=0; i<LOG_LINES_COUNT; i++)
        mvwprintw(log_window, line++, 0, logs[i]);
    wrefresh(log_window);
}

int main(void)
{
    server_pid = getpid();
    sd_init(&server_data);

    server_init_ncurses();
    server_init_sm();
    SERVER_ADD_LOG("Starting Server, pid=%d", server_pid);


    pthread_create(&display_thread, NULL, server_display_thread, NULL);
    pthread_create(&input_thread, NULL, server_input_thread, NULL);
    pthread_create(&update_thread, NULL, server_update_thread, NULL);
    pthread_join(input_thread, NULL);

    munmap(sm_block, SHARED_BLOCK_SIZE);
    close(fd);
    shm_unlink(SHM_FILE_NAME);
    endwin();
    return 0;
}