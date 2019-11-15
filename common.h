#ifndef __COMMON_H__
#define __COMMON_H__

#include <semaphore.h>

#define MAX_CLIENTS_COUNT 4

#define MAP_WIDTH 50
#define MAP_HEIGHT 30

#define SHM_FILE_NAME "game_shm"
#define SHARED_BLOCK_SIZE sizeof(struct clients_sm_block_t)

enum client_type_t { CLIENT_TYPE_CPU, CLIENT_TYPE_HUMAN, CLIENT_TYPE_FREE };
enum action_t { ACTION_GO_LEFT, ACTION_GO_RIGHT, ACTION_GO_UP, ACTION_GO_DOWN, ACTION_DO_NOTHING };

struct client_input_block_t
{
    enum action_t action;
} 
__attribute__((packed));

struct client_output_block_t
{
    int x;
    int y;

    int coins_found;
    int coins_brought;

    int deaths;
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

#endif