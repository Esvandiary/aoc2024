#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"
#include "astar.h"

#include <stdbool.h>

#define isdigit(c) ((c) >= '0' && (c) <= '9')

typedef struct pos
{
    uint32_t x;
    uint32_t y;
} pos;

static pos bestpath[1024];
static uint32_t bestpathcount;
static const uint32_t gw = 71;
static const uint32_t gh = 71;
static const int p1blockcount = 1024;

static astar_node nodes[16384];

static int64_t astarsteps(uint32_t sx, uint32_t sy, uint32_t ex, uint32_t ey)
{
    for (int y = 0; y < gh; ++y)
    {
        for (int x = 0; x < gw; ++x)
        {
            nodes[y*gw + x].fScore = INT64_MAX;
            nodes[y*gw + x].traversed = false;
        }
    }

    astar_node* end = calculate(
        nodes,
        gh,
        gw,
        (astar_pos) { .y = sy, .x = sx },
        (astar_pos) { .y = ey, .x = ex });

    if (!end)
        return -1;
    
    astar_node* n = end;
    for (int64_t i = end->fScore - 1; i >= 0 && n; --i, n = n->prev)
        bestpath[i] = (pos) { n->pos.x, n->pos.y };
    bestpathcount = end->fScore;

    return end->fScore;
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    const uint32_t sx = 0;
    const uint32_t sy = 0;
    const uint32_t ex = gw - 1;
    const uint32_t ey = gh - 1;

    for (int y = 0; y < gh; ++y)
    {
        for (int x = 0; x < gw; ++x)
        {
            nodes[y*gw + x] = (astar_node) {
                .pos = (astar_pos) {.y = y, .x = x},
                .blockcount = 0,
                .fScore = INT64_MAX,
                .prev = NULL,
                .traversed = false,
            };
        }
    }

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

        ++nodes[y*gw + x].blockcount;

        if (++blocks == p1blockcount)
        {
            DEBUGLOG("[P1] running\n");
            uint64_t sum1 = astarsteps(sx, sy, ex, ey);
            DEBUGLOG("[P1] done\n");
            print_int64(sum1);
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
                int64_t result = astarsteps(sx, sy, ex, ey);
                DEBUGLOG("[P2] new best = %" PRId64 "\n", result);
                if (result < 0)
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
