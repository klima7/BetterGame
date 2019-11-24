#include <ncursesw/ncurses.h>
#include <stdlib.h>
#include "map.h"
#include "common.h"
#include "tiles.h"

// Funkcje statyczne
static void map_maze_recur(struct map_t *map, int x, int y);
static void map_add_holes_in_maze(struct map_t *map);
static void map_add_bush(struct map_t *map);

// Funckcja zwracająca kafelek w danych miejscu (lub TILE_VOID)
enum tile_t map_get_tile(struct map_t *map, int x, int y)
{
    if(x<0 || y<0 || x>=MAP_WIDTH || y>=MAP_HEIGHT)
        return TILE_VOID;
    else 
        return map->map[y][x];
}

// Funkcja ustalająca dany kafelek na mapie (albo i nie)
void map_set_tile(struct map_t *map, int x, int y, enum tile_t tile)
{
    if(x<0 || y<0 || x>MAP_WIDTH || y>MAP_HEIGHT)
        return;
    else
        map->map[y][x] = tile;
}

// Funkcja wyświetlająca mapę w podanym oknie
void map_display(struct map_t *map, WINDOW *window)
{
    werase(window);

    // Wyświetla mape
    for(int i=0; i<MAP_VIEW_HEIGHT; i++)
    {
        for(int j=0; j<MAP_VIEW_WIDTH; j++)
        {
            int map_y = map->viewpoint_y + i;
            int map_x = map->viewpoint_x + j;

            enum tile_t tile = map_get_tile(map, map_x, map_y);
            const chtype color_character = tile_get_appearance(tile);

            int display_x = j+2;
            int display_y = i+1;

            if(MAP_WIDTH<MAP_VIEW_WIDTH) display_x += (MAP_VIEW_WIDTH-MAP_WIDTH)/2;
            if(MAP_HEIGHT<MAP_VIEW_HEIGHT) display_y += (MAP_VIEW_HEIGHT-MAP_HEIGHT)/2;

            mvwaddch(window, display_y, display_x, color_character);
        }
    }

    // Wyświetl ramke
    for(int i=0; i<MAP_VIEW_WIDTH+2; i++)
    {
        mvwaddch(window, MAP_VIEW_HEIGHT+1, i, ' '|COLOR_PAIR(COLOR_GREEN_ON_YELLOW));
        mvwaddch(window, 0, i, ' '|COLOR_PAIR(COLOR_GREEN_ON_YELLOW));
    }
    for(int i=0; i<MAP_VIEW_HEIGHT+2; i++)
    {
        mvwaddch(window, i, 0, ' '|COLOR_PAIR(COLOR_GREEN_ON_YELLOW));
        mvwaddch(window, i, 1, ' '|COLOR_PAIR(COLOR_GREEN_ON_YELLOW));
        mvwaddch(window, i, MAP_VIEW_WIDTH+2, ' '|COLOR_PAIR(COLOR_GREEN_ON_YELLOW));
        mvwaddch(window, i, MAP_VIEW_WIDTH+3, ' '|COLOR_PAIR(COLOR_GREEN_ON_YELLOW));
    }

    // Wyświetla poziomy pasek przewijania
    if(MAP_WIDTH>MAP_VIEW_WIDTH)
    {
        int viewpoint_max = MAP_WIDTH-MAP_VIEW_WIDTH;
        int pos = map->viewpoint_x*(MAP_VIEW_WIDTH-3)/viewpoint_max+2;
        mvwaddch(window, MAP_VIEW_HEIGHT+1, pos, ' '|COLOR_PAIR(COLOR_WHITE_ON_MAGENTA));
        mvwaddch(window, MAP_VIEW_HEIGHT+1, pos+1, ' '|COLOR_PAIR(COLOR_WHITE_ON_MAGENTA));
    }

    // Wyświetla pionowy pasek przewijania
    if(MAP_HEIGHT>MAP_VIEW_HEIGHT)
    {
        int viewpoint_max = MAP_HEIGHT-MAP_VIEW_HEIGHT;
        int pos = map->viewpoint_y*(MAP_VIEW_HEIGHT-1)/viewpoint_max+1;
        mvwaddch(window, pos, MAP_VIEW_WIDTH+2, ' '|COLOR_PAIR(COLOR_WHITE_ON_MAGENTA));
        mvwaddch(window, pos, MAP_VIEW_WIDTH+3, ' '|COLOR_PAIR(COLOR_WHITE_ON_MAGENTA));
    }

    wrefresh(window);
}

// Kopiowanie mapy
void map_copy(const struct map_t *source, struct map_t *destination)
{
    for(int i=0; i<MAP_HEIGHT; i++)
    {
        for(int j=0; j<MAP_WIDTH; j++)
        {
            destination->map[i][j] = source->map[i][j];
        }
    }
}

// Wypełnienie całej mapy podanym kafelkiem
void map_fill(struct map_t *map, enum tile_t tile)
{
    for(int i=0; i<MAP_HEIGHT; i++)
    {
        for(int j=0; j<MAP_WIDTH; j++)
            map->map[i][j] = tile;
    }
}

// Dodaje do mapy nowopoznany obszar
void map_update_with_surrounding_area(struct map_t *map, surrounding_area_t *area, int x, int y)
{
    for(int i=0; i<VISIBLE_AREA_SIZE; i++)
    {
        for(int j=0; j<VISIBLE_AREA_SIZE; j++)
        {
            enum tile_t tile = (*area)[i][j];
            if(tile==TILE_VOID) continue;
            
            int abs_y = y+i-VISIBLE_DISTANCE;
            int abs_x = x+j-VISIBLE_DISTANCE;

            if(tile==TILE_CAMPSIDE)
            {
                map->campside_x = abs_x;
                map->campside_y = abs_y;
            }

            map_set_tile(map, abs_x, abs_y, tile);
        }
    }
}

// Usówa kafelki które mogły zmienić swoją pozycję
void map_remove_unsure_tiles(struct map_t *map)
{
    for(int y=0; y<MAP_HEIGHT; y++)
    {
        for(int x=0; x<MAP_WIDTH; x++)
        {
            enum tile_t tile = map_get_tile(map, x, y);
            if(!tile_is_sure(tile)) 
                map_set_tile(map, x, y, TILE_FLOOR);
        }
    }
}

// Funkcja rekurencyjnego generowania labiryntu
static void map_maze_recur(struct map_t *map, int x, int y)
{
    int unchecked[4] = { MAP_GEN_LEFT, MAP_GEN_RIGHT, MAP_GEN_UP, MAP_GEN_DOWN };

    for(int i=4; i>0; i--)
    {
        int r = rand()%i;
        int dir = unchecked[r];
        unchecked[r] = unchecked[i-1];

        int dirx, diry;
        if(dir==MAP_GEN_UP)         { dirx=0;   diry=-1; }
        else if(dir==MAP_GEN_DOWN)  { dirx=0;   diry=1;  }
        else if(dir==MAP_GEN_LEFT)  { dirx=-1;  diry=0;  }
        else if(dir==MAP_GEN_RIGHT) { dirx=1;   diry=0;  }

        if(map_get_tile(map, x+dirx*2, y+diry*2)==TILE_WALL)
        {
            map_set_tile(map, x+dirx*1, y+diry*1, TILE_FLOOR);
            map_set_tile(map, x+dirx*2, y+diry*2, TILE_FLOOR);
            map_maze_recur(map, x+dirx*2, y+diry*2);
        }
    }
}

// Generuje labirynt
void map_generate_maze(struct map_t *map)
{
    map_fill(map, TILE_WALL);
    map_maze_recur(map, 1, 1);
}

// Scrolluje mapę w podanych kierunkach (lub nie)
void map_shift(struct map_t *map, int shift_x, int shift_y)
{
    if(MAP_WIDTH>MAP_VIEW_WIDTH)
    {
        map->viewpoint_x += shift_x;
        if(map->viewpoint_x<0)
            map->viewpoint_x = 0;
        else if(map->viewpoint_x>MAP_WIDTH-MAP_VIEW_WIDTH)
            map->viewpoint_x = MAP_WIDTH-MAP_VIEW_WIDTH;
    }

    if(MAP_HEIGHT>MAP_VIEW_HEIGHT)
    {
        map->viewpoint_y += shift_y;
        if(map->viewpoint_y<0)
            map->viewpoint_y = 0;
        else if(map->viewpoint_y>MAP_HEIGHT-MAP_VIEW_HEIGHT)
            map->viewpoint_y = MAP_HEIGHT-MAP_VIEW_HEIGHT;
    }
}

// Losuje wolny kafelek i zwraca jego pozycje (lub nie)
int map_random_free_position(struct map_t *map, int *resx, int *resy)
{
    int good_pos = 0;

    for(int i=0; i<MAP_HEIGHT; i++)
    {
        for(int j=0; j<MAP_WIDTH; j++)
        {
            if(map_get_tile(map, j, i)==TILE_FLOOR)
                good_pos++;
        }
    }

    // Wolnego kafelka nie udało się znaleźć
    if(good_pos==0) return 1;

    int pos = rand()%good_pos;

    for(int i=0; i<MAP_HEIGHT; i++)
    {
        for(int j=0; j<MAP_WIDTH; j++)
        {
            if(map_get_tile(map, j, i)==TILE_FLOOR)
            {
                if(pos==0)
                {
                    *resx = j;
                    *resy = i;
                    return 0;
                }
                pos--;
            }
        }
    }
    return 1;
}

// Dodaje do mapy krzaki
static void map_add_bush(struct map_t *map)
{
    int bush_count = MAP_WIDTH*MAP_HEIGHT/MAP_GEN_BUSH_FACTOR;

    for(int i=0; i<bush_count; i++)
    {
        int x = 0;
        int y = 0;
        int res = map_random_free_position(map, &x, &y);
        if(res!=0) return;
        map_set_tile(map, x, y, TILE_BUSH);
    }
}

// Dodaje do labiryntu losowe dziury w ścianach
static void map_add_holes_in_maze(struct map_t *map)
{
    for(int i=0; i<MAP_HEIGHT*MAP_WIDTH/MAP_GEN_HOLES_FACTOR; i++)
    {
        int x = rand()%(MAP_WIDTH/2-1)*2+2;
        int y = rand()%(MAP_HEIGHT/2-1)*2+2;
        map_set_tile(map, x, y, TILE_FLOOR);
    }
}

// Generuje mapę
void map_generate_everything(struct map_t *map)
{
    map_generate_maze(map);
    map_add_holes_in_maze(map);
    map_random_free_position(map, &map->campside_x, &map->campside_y);
    map_add_bush(map);
}