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

static uint64_t traverse_plot(map map, pos pos, dir indir)
{
    const char c = MAPCHARAT(pos.x, pos.y);
    const char lc = c | 0x20;
    MAPCHARAT(pos.x, pos.y) = lc;
    uint64_t result = (1ULL << 32);

    if (pos.y > 0)
    {
        const char upc = MAPCHARAT(pos.x, pos.y-1);
        if (upc == c)
            result += traverse_plot(map, (struct pos){pos.x, pos.y-1}, DOWN);
        else if (upc != lc)
            result += 1;
    }
    else
    {
        result += 1;
    }

    if (pos.x > 0)
    {
        const char upc = MAPCHARAT(pos.x-1, pos.y);
        if (upc == c)
            result += traverse_plot(map, (struct pos){pos.x-1, pos.y}, RIGHT);
        else if (upc != lc)
            result += 1;
    }
    else
    {
        result += 1;
    }

    if (pos.y+1 < map.height)
    {
        const char upc = MAPCHARAT(pos.x, pos.y+1);
        if (upc == c)
            result += traverse_plot(map, (struct pos){pos.x, pos.y+1}, UP);
        else if (upc != lc)
            result += 1;
    }
    else
    {
        result += 1;
    }

    if (pos.x+1 < map.width)
    {
        const char upc = MAPCHARAT(pos.x+1, pos.y);
        if (upc == c)
            result += traverse_plot(map, (struct pos){pos.x+1, pos.y}, LEFT);
        else if (upc != lc)
            result += 1;
    }
    else
    {
        result += 1;
    }

    return result;
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int width = 0;
    while (file.data[width] != '\n')
        ++width;
    int linewidth = width;
    while (!ISVALID(file.data[linewidth]))
        ++linewidth;
    int height = (fileSize+2) / linewidth;

    map map = (struct map) { file.data, linewidth, width, height };

    uint64_t sum1 = 0;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (ISVALIDAT(x, y))
            {
                DEBUGLOG("plot of %c", CHARAT(x, y));
                const uint64_t plotresult = traverse_plot(map, (struct pos) { x, y }, LEFT);
                const uint32_t area = (plotresult >> 32);
                const uint32_t perimeter = (plotresult & 0xFFFFFFFFULL);
                sum1 += area * perimeter;
                DEBUGLOG(" price %u x %u = %u\n", area, perimeter, area * perimeter);
            }
        }
    }

    print_uint64(sum1);

    return 0;
}
