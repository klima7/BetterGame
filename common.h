#ifndef __COMMON_H__
#define __COMMON_H__

#include <semaphore.h>
#include <ncursesw/ncurses.h>

#define COLOR_WHITE_ON_BLACK    1
#define COLOR_BLACK_ON_WHITE    2
#define COLOR_RED_ON_WHITE      3
#define COLOR_BLACK_ON_YELLOW   4
#define COLOR_WHITE_ON_MAGENTA  5
#define COLOR_GREEN_ON_YELLOW   6
#define COLOR_YELLOW_ON_GREEN   7

#define SMALL_TREASURE_VALUE 10
#define BIG_TREASURE_VALUE 50

#define MAX_CLIENTS_COUNT 4

#define MAP_WIDTH 50
#define MAP_HEIGHT 30

#define MAP_VIEW_WIDTH 50
#define MAP_VIEW_HEIGHT 30

#define VISIBLE_DISTANCE 2
#define VISIBLE_AREA_SIZE (VISIBLE_DISTANCE*2+1)

#define SHM_FILE_NAME "game_shm"
#define SHARED_BLOCK_SIZE sizeof(struct clients_sm_block_t)

enum tile_t
{
    TILE_VOID       = 0,
    TILE_WALL       = 1,
    TILE_FLOOR      = 2,
    TILE_CAMPSIDE   = 3,
    TILE_BUSH       = 4,
    TILE_BEAST      = 5,
    TILE_COIN       = 6,
    TILE_S_TREASURE = 7,
    TILE_L_TREASURE = 8,
    TILE_DROP       = 9,
    TILE_UNKNOWN    = 10,

    TILE_PLAYER1    = 11,
    TILE_PLAYER2    = 12,
    TILE_PLAYER3    = 13,
    TILE_PLAYER4    = 14
};

enum client_type_t 
{ 
    CLIENT_TYPE_CPU, 
    CLIENT_TYPE_HUMAN, 
    CLIENT_TYPE_FREE 
};

enum action_t 
{ 
    
    ACTION_GO_LEFT, 
    ACTION_GO_RIGHT, 
    ACTION_GO_UP, 
    ACTION_GO_DOWN, 
    ACTION_DO_NOTHING 
};

typedef enum tile_t surrounding_area_t[VISIBLE_AREA_SIZE][VISIBLE_AREA_SIZE];

struct client_input_block_t
{
    enum action_t action;
} 
__attribute__((packed));

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

struct client_data_block_t
{
    int client_pid;
    enum client_type_t client_type;
}
__attribute__((packed));

struct client_sm_block_t
{
    sem_t data_cs;

    struct client_data_block_t data_block;

    struct client_input_block_t input_block;

    struct client_output_block_t output_block;
    sem_t output_block_sem;
} 
__attribute__((packed));

struct clients_sm_block_t
{
    struct client_sm_block_t clients[MAX_CLIENTS_COUNT];
} 
__attribute__((packed));

void check(int expr, const char *message);
void display_center(const char *message);
void init_colors(void);

#endif