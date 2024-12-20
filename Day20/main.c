// #define ENABLE_DEBUGLOG
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <math.h>

#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) < (y) ? (y) : (x))

#define POSX(idx) ((idx) % linewidth)
#define POSY(idx) ((idx) / linewidth)
#define IDXOF(x,y) ((y) * linewidth + (x))
#define CHARAT(x,y) file.data[IDXOF((x),(y))]

typedef enum dir
{
    UP = 0,
    RIGHT = 1,
    DOWN = 2,
    LEFT = 3
} dir;

typedef struct block
{
    uint16_t clear : 1;
    uint16_t count : 15;
    int8_t ndx;
    int8_t ndy;
} block;

static block blockinfo[256][256];

static inline FORCEINLINE uint32_t check_shortcut_p1(int w, int h, int x, int y, int dx, int dy)
{
    if (x + 2*dx < 0 || x + 2*dx >= w || y + 2*dy < 0 || y + 2*dy >= h)
        return 0;

    if (!blockinfo[y+dy][x+dx].clear && blockinfo[y+2*dy][x+2*dx].clear)
    {
        int what_a_savings = (int)blockinfo[y+2*dy][x+2*dx].count - blockinfo[y][x].count - 2;
        if (what_a_savings > 0)
        {
            // shortcut
            DEBUGLOG("shortcut [%d,%d] -> [%d,%d] saves %d\n", x, y, x+2*dx, y+2*dy, what_a_savings);
            return what_a_savings;
        }
    }

    return 0;
}

static inline FORCEINLINE uint32_t check_shortcut_p2(int w, int h, int x, int y)
{
    uint32_t savings_count = 0;
    int startx = max(0, x - 20);
    int endx = min(x + 20, w - 1);
    for (int cx = startx; cx <= endx; ++cx)
    {
        int remaining = abs(x - cx);
        int starty = max(0, y - 20 + remaining);
        int endy = max(0, y + 20 - remaining);
        for (int cy = starty; cy <= endy; ++cy)
        {
            int what_a_savings = blockinfo[cy][cx].count - blockinfo[y][x].count - abs(x - cx) - abs(y - cy);
            if (what_a_savings >= 100)
                ++savings_count;
        }
    }
    return savings_count;
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int width = 0;
    while (file.data[width] != '\n')
        ++width;
    int linewidth = width;
    while (file.data[linewidth] == '\n')
        ++linewidth;
    int height = (fileSize+2) / linewidth;

    int idx = linewidth;
    int sx = -1, sy =-1, ex = -1, ey = -1;
    while (idx < fileSize)
    {
        if (file.data[idx] == 'S')
        {
            sx = POSX(idx);
            sy = POSY(idx);
            if (ex != -1)
                break;
        }
        else if (file.data[idx] == 'E')
        {
            ex = POSX(idx);
            ey = POSY(idx);
            if (sx != -1)
                break;
        }
        ++idx;
    }

    DEBUGLOG("grid [%dx%d] start [%d,%d] end [%d,%d]\n", width, height, sx, sy, ex, ey);

    int x = sx, y = sy, count = 0;
    dir lastdir = -1;
    while (x != ex || y != ey)
    {
        blockinfo[y][x].clear = 1;
        blockinfo[y][x].count = count++;
        if (lastdir != DOWN && CHARAT(x,y-1) != '#')
        {
            blockinfo[y][x].ndy = -1;
            lastdir = UP;
            y -= 1;
        }
        else if (lastdir != LEFT && CHARAT(x+1,y) != '#')
        {
            blockinfo[y][x].ndx = 1;
            lastdir = RIGHT;
            x += 1;
        }
        else if (lastdir != UP && CHARAT(x,y+1) != '#')
        {
            blockinfo[y][x].ndy = 1;
            lastdir = DOWN;
            y += 1;
        }
        else if (lastdir != RIGHT && CHARAT(x-1,y) != '#')
        {
            blockinfo[y][x].ndx = -1;
            lastdir = LEFT;
            x -= 1;
        }
    }
    blockinfo[ey][ex].clear = 1;
    blockinfo[ey][ex].count = count;

    uint64_t sum1 = 0, sum2 = 0;

    x = sx, y = sy, count = 0;
    while (x != ex || y != ey)
    {
        if (check_shortcut_p1(width, height, x, y, 0, -1) >= 100)
            ++sum1;
        if (check_shortcut_p1(width, height, x, y, 1, 0) >= 100)
            ++sum1;
        if (check_shortcut_p1(width, height, x, y, 0, 1) >= 100)
            ++sum1;
        if (check_shortcut_p1(width, height, x, y, -1, 0) >= 100)
            ++sum1;

        sum2 += check_shortcut_p2(width, height, x, y);

        int8_t dx = blockinfo[y][x].ndx;
        int8_t dy = blockinfo[y][x].ndy;
        x += dx;
        y += dy;
    }

    print_uint64(sum1);
    print_uint64(sum2);

    return 0;
}
