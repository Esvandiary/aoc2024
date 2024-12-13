#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#define INDEXOF(x,y) ((y) * (linewidth) + (x))
#define CHARAT(x,y) (file.data[(y)*linewidth + (x)])
#define MAPINDEXOF(x,y) ((y) * (map.linewidth) + (x))
#define MAPCHARAT(x,y) (map.data[MAPINDEXOF((x),(y))])

#define ISVALID(c) ((c) >= 'A' && (c) <= 'Z')

typedef struct map
{
    char* data;
    uint16_t linewidth;
    uint16_t width;
    uint16_t height;
} map;

typedef struct pos
{
    int32_t x;
    int32_t y;
} pos;

typedef enum dir
{
    UP,
    RIGHT,
    DOWN,
    LEFT
} dir;

#define INVALID '~'

static uint32_t areas[4096];
static uint8_t cornercount[65536];

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
    uint64_t result = (1ULL << 32) | ((uint64_t)cornercount[MAPINDEXOF(pos.x, pos.y)] << 48);

    CHECK_DIR(pos.x+1 < map.width, pos.x+1, pos.y);
    CHECK_DIR(pos.y > 0, pos.x, pos.y-1);
    CHECK_DIR(pos.x > 0, pos.x-1, pos.y);
    CHECK_DIR(pos.y+1 < map.height, pos.x, pos.y+1);

    return result;
}

static inline FORCEINLINE uint8_t get_corner_count(map map, pos pos)
{
    const char c11 = MAPCHARAT(pos.x, pos.y);
    const char c00 = (pos.x > 0 && pos.y > 0 ? MAPCHARAT(pos.x-1, pos.y-1) : '~');
    const char c01 = (pos.y > 0 ? MAPCHARAT(pos.x, pos.y-1) : '~');
    const char c02 = (pos.x+1 < map.width && pos.y > 0 ? MAPCHARAT(pos.x+1, pos.y-1) : '~');
    const char c10 = (pos.x > 0 ? MAPCHARAT(pos.x-1, pos.y) : '~');
    const char c12 = (pos.x+1 < map.width ? MAPCHARAT(pos.x+1, pos.y) : '~');
    const char c20 = (pos.x > 0 && pos.y+1 < map.height ? MAPCHARAT(pos.x-1, pos.y+1) : '~');
    const char c21 = (pos.y+1 < map.height ? MAPCHARAT(pos.x, pos.y+1) : '~');
    const char c22 = (pos.x+1 < map.width && pos.y+1 < map.height ? MAPCHARAT(pos.x+1, pos.y+1) : '~');

    uint8_t result = 0;
    // inner corners
    if (c10 != c11 && c01 != c11) ++result;
    if (c12 != c11 && c01 != c11) ++result;
    if (c10 != c11 && c21 != c11) ++result;
    if (c12 != c11 && c21 != c11) ++result;
    // outer corners
    if (c10 == c11 && c01 == c11 && c00 != c11) ++result;
    if (c12 == c11 && c01 == c11 && c02 != c11) ++result;
    if (c10 == c11 && c21 == c11 && c20 != c11) ++result;
    if (c12 == c11 && c21 == c11 && c22 != c11) ++result;

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

    uint64_t sum1 = 0, sum2 = 0;

    for (int x = 1; x < width - 1; ++x)
    {
        cornercount[INDEXOF(x, 0)] = get_corner_count(map, (struct pos) { x, 0 });
        cornercount[INDEXOF(x, height-1)] = get_corner_count(map, (struct pos) { x, height-1 });
    }

    for (int y = 0; y < height; ++y)
    {
        cornercount[INDEXOF(0, y)] = get_corner_count(map, (struct pos) { 0, y });
        if (y > 0)
            cornercount[INDEXOF(0, y-1)] = get_corner_count(map, (struct pos) { 0, y-1 });
        if (y+1 < height)
            cornercount[INDEXOF(0, y+1)] = get_corner_count(map, (struct pos) { 0, y+1 });

        for (int x = 1; x < width; ++x)
        {
            if (CHARAT(x, y) != CHARAT(x-1, y))
            {
                cornercount[INDEXOF(x-1, y)] = get_corner_count(map, (struct pos) { x-1, y });
                cornercount[INDEXOF(x, y)] = get_corner_count(map, (struct pos) { x, y });
                if (y > 0)
                {
                    cornercount[INDEXOF(x-1, y-1)] = get_corner_count(map, (struct pos) { x-1, y-1 });
                    cornercount[INDEXOF(x, y-1)] = get_corner_count(map, (struct pos) { x, y-1 });
                }
                if (y+1 < height)
                {
                    cornercount[INDEXOF(x-1, y+1)] = get_corner_count(map, (struct pos) { x-1, y+1 });
                    cornercount[INDEXOF(x, y+1)] = get_corner_count(map, (struct pos) { x, y+1 });
                }
            }
        }
        cornercount[INDEXOF(width-1, y)] = get_corner_count(map, (struct pos) { width-1, y });
        if (y > 0)
            cornercount[INDEXOF(width-1, y-1)] = get_corner_count(map, (struct pos) { width-1, y-1 });
        if (y+1 < height)
            cornercount[INDEXOF(width-1, y+1)] = get_corner_count(map, (struct pos) { width-1, y+1 });
    }

#if defined(ENABLE_DEBUGLOG)
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
            DEBUGLOG("%d", cornercount[INDEXOF(x,y)]);
        DEBUGLOG("\n");
    }
#endif

    int areaidx = 0;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (ISVALID(CHARAT(x, y)))
            {
                DEBUGLOG("plot of %c:\n", CHARAT(x, y));
                const uint64_t plotresult = traverse_plot(map, (struct pos) { x, y });
                const uint32_t area = (plotresult >> 32) & 0xFFFFULL;
                const uint32_t perimeter = (plotresult & 0xFFFFFFFFULL);
                const uint32_t sides = (plotresult >> 48);
                sum1 += area * perimeter;
                sum2 += area * sides;
                DEBUGLOG("    P1 price %u x %u = %u, P2 price %u x %u = %u\n", area, perimeter, area * perimeter, area, sides, area * sides);
            }
        }
    }

    print_uint64(sum1);
    print_uint64(sum2);

    return 0;
}
