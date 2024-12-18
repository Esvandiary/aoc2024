// #define ENABLE_DEBUGLOG
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

static uint32_t grid[71][71];
static uint32_t gridbest[71][71];
static const uint32_t gw = 71;
static const uint32_t gh = 71;
static const int blockcount = 1024;

typedef enum dir
{
    UP,
    RIGHT,
    DOWN,
    LEFT
} dir;

static void p1steps(uint32_t x, uint32_t y, uint32_t ex, uint32_t ey, uint32_t steps)
{
    if (!gridbest[y][x] || steps < gridbest[y][x])
        gridbest[y][x] = steps;
    else
        return;
    if (x == ex && y == ey)
        return;

    if (x > 0 && !grid[y][x-1])
        p1steps(x-1, y, ex, ey, steps + 1);
    if (y > 0 && !grid[y-1][x])
        p1steps(x, y-1, ex, ey, steps + 1);
    if (x+1 < gw && !grid[y][x+1])
        p1steps(x+1, y, ex, ey, steps + 1);
    if (y+1 < gh && !grid[y+1][x])
        p1steps(x, y+1, ex, ey, steps + 1);
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    const uint32_t sx = 0;
    const uint32_t sy = 0;
    const uint32_t ex = gw - 1;
    const uint32_t ey = gh - 1;

    int idx = 0;
    int blocks = 0;
    while (idx < fileSize - 1)
    {
        uint32_t x = 0;
        while (isdigit(file.data[idx]))
            x = (x * 10) + (file.data[idx++] & 0xF);
        ++idx; // ','
        uint32_t y = 0;
        while (isdigit(file.data[idx]))
            y = (y * 10) + (file.data[idx++] & 0xF);
        ++idx; // '\n'

        ++grid[y][x];

        if (++blocks == blockcount)
            break;
    }

#if defined(ENABLE_DEBUGLOG)
    for (int y = 0; y < gh; ++y)
    {
        for (int x = 0; x < gw; ++x)
            DEBUGLOG("%c", grid[y][x] ? '#' : '.');
        DEBUGLOG("\n");
    }
#endif

    p1steps(sx, sy, ex, ey, 1);

    uint64_t sum1 = gridbest[ey][ex] - 1;
    print_uint64(sum1);

    return 0;
}
