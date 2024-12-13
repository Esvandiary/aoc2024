// #define ENABLE_DEBUGLOG
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#define CHARAT(x,y) (file.data[(y)*linewidth + (x)])
#define MAPCHARAT(x,y) (map.data[(y)*map.linewidth + (x)])

#define ISVALID(c) ((c) >= 'A' && (c) <= 'Z')
#define ISVALIDAT(x,y) ISVALID(CHARAT(x,y))
#define MAPISVALIDAT(x,y) ISVALID(MAPCHARAT(x,y))

typedef struct map
{
    char* data;
    uint16_t linewidth;
    uint16_t width;
    uint16_t height;
} map;

typedef struct pos
{
    uint32_t x;
    uint32_t y;
} pos;

typedef enum dir
{
    UP,
    RIGHT,
    DOWN,
    LEFT
} dir;

#define INVALID '~'

#define CHECK_DIR(reqt, x, y) { \
    if (reqt) { \
        const char upc = MAPCHARAT((x), (y)); \
        if (upc == c) \
            result += traverse_plot(map, (struct pos){(x), (y)}); \
        else if (upc != lc) \
            result += 1; \
    } \
    else \
    { \
        result += 1; \
    } \
}

static uint64_t traverse_plot(map map, pos pos)
{
    const char c = MAPCHARAT(pos.x, pos.y);
    const char lc = c | 0x20;
    MAPCHARAT(pos.x, pos.y) = lc;
    uint64_t result = (1ULL << 32);

    CHECK_DIR(pos.x+1 < map.width, pos.x+1, pos.y);
    CHECK_DIR(pos.y > 0, pos.x, pos.y-1);
    CHECK_DIR(pos.x > 0, pos.x-1, pos.y);
    CHECK_DIR(pos.y+1 < map.height, pos.x, pos.y+1);

    return result;
}


static uint32_t find_corners(const map map, pos opos, char c)
{
    pos cpos = opos;
    dir ctrack = LEFT;

    uint32_t corners = 0;

    int iters = 0;
    do
    {
        // check for out-corners
        if (ctrack == LEFT && cpos.x != 0 && MAPCHARAT(cpos.x-1, cpos.y) == c)
        {
            ++corners;
            ctrack = DOWN;
        }
        else if (ctrack == DOWN && cpos.y+1 < map.height && MAPCHARAT(cpos.x, cpos.y+1) == c)
        {
            ++corners;
            ctrack = RIGHT;
        }
        else if (ctrack == RIGHT && cpos.x+1 < map.width && MAPCHARAT(cpos.x+1, cpos.y) == c)
        {
            ++corners;
            ctrack = UP;
        }
        else if (ctrack == UP && cpos.y != 0 && MAPCHARAT(cpos.x, cpos.y-1) == c)
        {
            ++corners;
            ctrack = LEFT;
        }

        // try to move
        pos npos;
        if (ctrack == LEFT)
            npos = (pos) { cpos.x, cpos.y - 1 };
        else if (ctrack == UP)
            npos = (pos) { cpos.x + 1, cpos.y };
        else if (ctrack == RIGHT)
            npos = (pos) { cpos.x, cpos.y + 1 };
        else if (ctrack == DOWN)
            npos = (pos) { cpos.x - 1, cpos.y };

        if (npos.x >= 0 && npos.x < map.width && npos.y >= 0 && npos.y < map.height && MAPCHARAT(npos.x, npos.y) == c)
        {
            // success
            cpos = npos;
            continue;
        }
        else
        {
            // in-corner
            ++corners;
            ctrack = (ctrack + 1) & 3;
        }
    } while (!(cpos.x == opos.x && cpos.y == opos.y && ctrack == LEFT));
    
    return corners;
}


int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("sample.txt");
    const int fileSize = (int)(file.size);

    int width = 0;
    while (file.data[width] != '\n')
        ++width;
    int linewidth = width;
    while (!ISVALID(file.data[linewidth]))
        ++linewidth;
    int height = (fileSize+2) / linewidth;

    map map = (struct map) { file.data, linewidth, width, height };

    uint64_t sum1 = 0, sum2 = 0;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (ISVALIDAT(x, y))
            {
                DEBUGLOG("plot of %c", CHARAT(x, y));
                const uint64_t plotresult = traverse_plot(map, (struct pos) { x, y });
                const uint32_t area = (plotresult >> 32);
                const uint32_t perimeter = (plotresult & 0xFFFFFFFFULL);
                const uint32_t sides = find_corners(map, (struct pos) { x, y }, CHARAT(x, y));
                sum1 += area * perimeter;
                sum2 += area * sides;
                DEBUGLOG(" P1 price %u x %u = %u, P2 price = %u x %u = %u\n", area, perimeter, area * perimeter, area, sides, area * sides);
            }
        }
    }

    print_uint64(sum1);
    print_uint64(sum2);

    return 0;
}
