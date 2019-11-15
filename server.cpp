#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <ncursesw/ncurses.h>
#include "common.h"

#define LOG_LINES_COUNT 5
#define LOG_LINE_WIDTH 20

int fd;
struct clients_sm_block_t *sm_block;

WINDOW *stat_window;
WINDOW *log_window;

char logs[LOG_LINES_COUNT][LOG_LINE_WIDTH+1];

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

        sem_init(&client_block->data_block_cs, 1, 1);
        sem_init(&client_block->input_block_cs, 1, 1);
        sem_init(&client_block->output_block_sem, 1, 0);
    }
}

void server_display_stats(void)
{
    mvwprintw(stat_window, 0, 0, "Servers PID  : %d", 0);
    mvwprintw(stat_window, 1, 0, "Campside X/Y : %d/%d", 0, 0);
    mvwprintw(stat_window, 2, 0, "Round Number : %d", 0);

    int line = 5;

    for(int i=0; i<MAX_CLIENTS_COUNT; i++)
    {
        client_sm_block_t *client_block = sm_block->clients+i;

        sem_wait(&client_block->data_block_cs);
        enum client_type_t type = client_block->data_block.client_type;
        int pid = client_block->data_block.client_pid;
        sem_post(&client_block->data_block_cs);

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
            mvwprintw(stat_window, line++, 0, "PID:    %d", pid);
            mvwprintw(stat_window, line++, 0, "Pos:    %d/%d", 0, 0);
            mvwprintw(stat_window, line++, 0, "Deaths: %d", 0);
            mvwprintw(stat_window, line++, 0, "Coins:  %d/%d", 0, 0);
            line++;
        }
    }
    wrefresh(stat_window);
}

void server_add_log(char *log)
{
    for(int i=1; i<LOG_LINES_COUNT; i++)
        strncpy(logs[i-1], logs[i], LOG_LINE_WIDTH);
    strncpy(logs[LOG_LINES_COUNT-1], log, LOG_LINE_WIDTH);
}

void server_display_logs(void)
{
    int line=0;
    mvwprintw(log_window, line++, 0, "--------Logs--------");
    for(int i=0; i<LOG_LINES_COUNT; i++)
        mvwprintw(log_window, line++, 0, logs[i]);
    wrefresh(log_window);
}

int main(void)
{
    server_init_ncurses();
    server_init_sm();
    server_add_log((char*)"Starting Server");

    server_display_stats();
    server_display_logs();
    getchar();


    munmap(sm_block, SHARED_BLOCK_SIZE);
    close(fd);
    shm_unlink(SHM_FILE_NAME);
    endwin();
    return 0;
}