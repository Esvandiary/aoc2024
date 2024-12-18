#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <stdbool.h>

#define isdigit(c) ((c) >= '0' && (c) <= '9')

typedef struct pos
{
    uint32_t x;
    uint32_t y;
} pos;

static uint32_t grid[71][71];
static uint32_t gridbest[71][71];
static pos bestpath[1024];
static uint32_t bestpathcount;
static const uint32_t gw = 71;
static const uint32_t gh = 71;
static const int p1blockcount = 1024;

static bool p1steps(uint32_t x, uint32_t y, uint32_t ex, uint32_t ey, uint32_t steps)
{
    // DEBUGLOG("[%u] running for %u,%u\n", steps, x, y);
    if (!gridbest[y][x] || steps < gridbest[y][x])
        gridbest[y][x] = steps;
    else
        return false;

    if (x == ex && y == ey)
    {
        bestpath[steps-1] = (pos) { x, y };
        bestpathcount = steps;
        return true;
    }

    bool result = false;
    if (x > 0 && !grid[y][x-1])
        result = p1steps(x-1, y, ex, ey, steps + 1) || result;
    if (y > 0 && !grid[y-1][x])
        result = p1steps(x, y-1, ex, ey, steps + 1) || result;
    if (x+1 < gw && !grid[y][x+1])
        result = p1steps(x+1, y, ex, ey, steps + 1) || result;
    if (y+1 < gh && !grid[y+1][x])
        result = p1steps(x, y+1, ex, ey, steps + 1) || result;

    if (result)
        bestpath[steps-1] = (pos) { x, y };
    return result;
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

        if (++blocks == p1blockcount)
        {
            DEBUGLOG("[P1] running\n");
            p1steps(sx, sy, ex, ey, 1);
            DEBUGLOG("[P1] done\n");
            uint64_t sum1 = gridbest[ey][ex] - 1;
            print_uint64(sum1);

#if defined(ENABLE_DEBUGLOG)
            for (int y = 0; y < gh; ++y)
            {
                for (int x = 0; x < gw; ++x)
                    DEBUGLOG("%c", grid[y][x] ? '#' : '.');
                DEBUGLOG("\n");
            }
#endif
        }
        else if (blocks > p1blockcount)
        {
            bool breaksbest = false;
            for (int i = 0; i < bestpathcount; ++i)
            {
                if (x == bestpath[i].x && y == bestpath[i].y)
                {
                    breaksbest = true;
                    break;
                }
            }

            if (breaksbest)
            {
                DEBUGLOG("[P2] block @ %u,%u breaks best, reticulating splines\n", x, y);
                memset(gridbest, 0, sizeof(gridbest));
                if (!p1steps(sx, sy, ex, ey, 1))
                {
                    DEBUGLOG("[P2] done\n");
                    printf("%u,%u\n", x, y);
                    break;
                }
            }
        }
    }

    return 0;
}
