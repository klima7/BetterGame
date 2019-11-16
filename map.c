#include <ncursesw/ncurses.h>
#include "map.h"
#include "common.h"

static const struct color_char_t map_chars_to_display[] =
{
    {' ', COLOR_BLACK_ON_WHITE  },
    {' ', COLOR_WHITE_ON_BLACK  },
    {' ', COLOR_BLACK_ON_WHITE  },
    {'A', COLOR_YELLOW_ON_GREEN },
    {'#', COLOR_BLACK_ON_WHITE  },
    {'*', COLOR_RED_ON_WHITE    },
    {'c', COLOR_BLACK_ON_YELLOW },
    {'t', COLOR_BLACK_ON_YELLOW },
    {'T', COLOR_BLACK_ON_YELLOW },
    {'D', COLOR_GREEN_ON_YELLOW },
    {'?', COLOR_BLACK_ON_WHITE  }
};

struct color_char_t map_get_color_char_from_file(enum tile_t tile)
{
    int index = (int)tile;
    return map_chars_to_display[index];
}