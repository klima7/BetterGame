#ifndef __COMMON_H__
#define __COMMON_H__

#include <semaphore.h>
#include <assert.h>
#include <ncursesw/ncurses.h>
#include "tiles.h"

// Kolory
#define COLOR_WHITE_ON_BLACK    1
#define COLOR_BLACK_ON_WHITE    2
#define COLOR_RED_ON_WHITE      3
#define COLOR_BLACK_ON_YELLOW   4
#define COLOR_WHITE_ON_MAGENTA  5
#define COLOR_GREEN_ON_YELLOW   6
#define COLOR_YELLOW_ON_GREEN   7
#define COLOR_WHITE_ON_RED      8

// Ile monet znajduje się w skarbach
#define SMALL_TREASURE_VALUE 10
#define BIG_TREASURE_VALUE 50

// Maksymalna liczba graczy
#define MAX_CLIENTS_COUNT 4

// Rozmiar mapy
#define MAP_WIDTH 127
#define MAP_HEIGHT 127

// Rozmiary mapy muszą być nieparzyste by algorym generacji labiryntu działał
static_assert(MAP_WIDTH%2==1, "MAP_WIDTH must be odd");
static_assert(MAP_HEIGHT%2==1, "MAP_HEIGHT must be odd");

// Rozmiar widocznego okna, gdy większe od mapy to pojawiają się suwaki
#define MAP_VIEW_WIDTH 90
#define MAP_VIEW_HEIGHT 40

// Odległość widzenia gracza
#define VISIBLE_DISTANCE 2
#define VISIBLE_AREA_SIZE (VISIBLE_DISTANCE*2+1)

// Pamięć współdzielona
#define SHM_FILE_NAME "game_shm"
#define SHARED_BLOCK_SIZE sizeof(struct clients_sm_block_t)

// Czas trwania jednej tury
#define TURN_TIME 250000

// Maksymalny czas czekana na dostanie się do sekcji krytycznej - po tym czasei resetujemy sekcje
#define CS_WAITING_TIME_MAX 100

// Maksymalny margines czas czekania na dane od serwerza - po tym czasie uznajemy że serwer nie odpowiada
#define DATA_WAITING_TIME_MAX 1000000

// Typ klienta
enum client_type_t 
{ 
    CLIENT_TYPE_CPU, 
    CLIENT_TYPE_HUMAN, 
    CLIENT_TYPE_FREE 
};

// Możliwe akcje podejmowane przez klienta
enum action_t 
{ 
    
    ACTION_GO_LEFT, 
    ACTION_GO_RIGHT, 
    ACTION_GO_UP, 
    ACTION_GO_DOWN, 
    ACTION_DO_NOTHING,

    // Specialna flaga
    ACTION_VOID
};

// Najbliższe otoczenie gracza o promieniu VISIBLE_DISTANCE wysyłane klientom przez serwer
typedef enum tile_t surrounding_area_t[VISIBLE_AREA_SIZE][VISIBLE_AREA_SIZE];

// Dane wstawiane przez klienta, a odczytywane przez serwer
struct client_input_block_t
{
    enum action_t action;
    int respond_flag;
} 
__attribute__((packed));

// Dane wstawiane przez serwer, a odczytywane przez klienta
struct client_output_block_t
{
    int x;
    int y;

    surrounding_area_t surrounding_area;

    int coins_found;
    int coins_brought;

    int deaths;

    int round;
    int server_pid;
} 
__attribute__((packed));

// Dane używane przez serwer i klienta jednocześnie
struct client_data_block_t
{
    int client_pid;
    enum client_type_t client_type;
}
__attribute__((packed));

// Dane dla jednego klienta umieszczone w pamięci współdzielonej
struct client_sm_block_t
{
    sem_t data_cs;
    struct client_data_block_t data_block;
    struct client_input_block_t input_block;
    struct client_output_block_t output_block;
    sem_t output_block_sem;
} 
__attribute__((packed));

// Dane wszystkich klientów umieszczone w pamięci współdzielonej
struct clients_sm_block_t
{
    struct client_sm_block_t clients[MAX_CLIENTS_COUNT];
} 
__attribute__((packed));

// Prototypy funkcji
void check(int expr, const char *message);
void display_center(const char *message);
void init_colors(void);
enum action_t reverse_direction(enum action_t direction);
void enter_cs(sem_t *sem);
void exit_cs(sem_t *sem);

#endif
