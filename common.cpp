#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "common.h"
#include "ncursesw/ncurses.h"

// Jeżeli warunek jest nieprawidłowy to wyświetla wiadomość na środku ekranu i kończy
void check(int expr, const char *message)
{
    if(!expr)
    {
        display_center(message);
        getchar();
        endwin();
        exit(1);
    }
}

// Wyświetla wiadomość na środku ekranu
void display_center(const char *message)
{
        int w = getmaxx(stdscr);
        int h = getmaxy(stdscr);
        clear();
        mvprintw(h/2, (w-strlen(message))/2, message);
        refresh();
}

// Inicjuje pary kolorów
void init_colors(void)
{
    start_color();

    init_pair(COLOR_WHITE_ON_BLACK, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_BLACK_ON_WHITE, COLOR_BLACK, COLOR_WHITE);
    init_pair(COLOR_RED_ON_WHITE, COLOR_RED, COLOR_WHITE);
    init_pair(COLOR_BLACK_ON_YELLOW, COLOR_BLACK, COLOR_YELLOW);
    init_pair(COLOR_WHITE_ON_MAGENTA, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(COLOR_GREEN_ON_YELLOW, COLOR_GREEN, COLOR_YELLOW);
    init_pair(COLOR_YELLOW_ON_GREEN, COLOR_YELLOW, COLOR_GREEN);
    init_pair(COLOR_WHITE_ON_RED, COLOR_WHITE, COLOR_RED);
}

// Zwraca kierunek przeciwny do podanego
enum action_t reverse_direction(enum action_t direction)
{
    if(direction==ACTION_GO_DOWN) return ACTION_GO_UP;
    else if(direction==ACTION_GO_UP) return ACTION_GO_DOWN;
    else if(direction==ACTION_GO_LEFT) return ACTION_GO_RIGHT;
    else if(direction==ACTION_GO_RIGHT) return ACTION_GO_LEFT;
    return direction;
}

// Wejście do sekcji krytycznej - czekanie na semaforze z pewnymi zabezpieczeniami, gdyby któryś klient wyszedł wcześniej nienaturalnie w złym momencie
void enter_cs(sem_t *sem)
{
    struct timespec tolerated_time;
    clock_gettime(CLOCK_REALTIME, &tolerated_time);
    tolerated_time.tv_sec += CS_WAITING_TIME_MAX / 1000000;
    long temp_ns = tolerated_time.tv_nsec + CS_WAITING_TIME_MAX%1000000*1000;
    tolerated_time.tv_sec += temp_ns / 1000000000;
    tolerated_time.tv_nsec = temp_ns % 1000000000;
    int res = sem_timedwait(sem, &tolerated_time);
    if(res!=0)
    {
        // Naprawienie sekcji krytycznej
        sem_destroy(sem);
        sem_init(sem, 1, 1);
    }
}

// Wyjście z sekcji krytycznej - para do enter_cs
void exit_cs(sem_t *sem)
{
    sem_post(sem);
}