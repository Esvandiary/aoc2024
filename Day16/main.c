#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <stdbool.h>
#include <nmmintrin.h>

#define POSX(idx) ((idx) % linewidth)
#define POSY(idx) ((idx) / linewidth)
#define IDXOF(x,y) ((y)*linewidth + (x))

typedef enum dir
{
    UP = 0,
    RIGHT = 1,
    DOWN = 2,
    LEFT = 3
} dir;

static const char* dirnames[] = {
    "up",
    "right",
    "down",
    "left"
};

static uint32_t grid[160][160][4];
static uint32_t best = 0;

static uint64_t gridfast[160][3];

static bool fill(const char* data, int linewidth, int x, int y, dir d, uint32_t score)
{
    const char cur = data[IDXOF(x,y)];
    if (cur == '#')
        return false;
    if (cur == 'E')
    {
        if (best && score > best)
            return false;
        if (!best || score < best)
        {
            DEBUGLOG("[END] score %u beats %u, clearing grid\n", score, best);
            memset(gridfast, 0, sizeof(gridfast));
            gridfast[y][x >> 6] |= ((uint64_t)1 << (x & 0x3F));
        }
        best = score;
        return true;
    }

    if (grid[y][x][d] == 0 || grid[y][x][d] >= score)
    {
        grid[y][x][d] = score;

        bool result = false;
        if (d != RIGHT)
            result = fill(data, linewidth, x-1, y, LEFT, score + (d == LEFT ? 1 : 1001)) || result;
        if (d != DOWN)
            result = fill(data, linewidth, x, y-1, UP, score + (d == UP ? 1 : 1001)) || result;
        if (d != LEFT)
            result = fill(data, linewidth, x+1, y, RIGHT, score + (d == RIGHT ? 1 : 1001)) || result;
        if (d != UP)
            result = fill(data, linewidth, x, y+1, DOWN, score + (d == DOWN ? 1 : 1001)) || result;
        
        if (result)
            gridfast[y][x >> 6] |= ((uint64_t)1 << (x & 0x3F));

#if defined(SPAMSPAMSPAM)
        DEBUGLOG("fill(%d,%d,%s,%u) --> %s\n", x, y, dirnames[d], score, result ? "Y" : "N");
#endif
        return result;
    }
    else
    {
#if defined(SPAMSPAMSPAM)
        DEBUGLOG("fill(%d,%d,%s,%u) --> WORSE\n", x, y, dirnames[d], score);
#endif
        return false;
    }
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int idx = 0;
    while (file.data[idx] != '\n')
        ++idx;
    const int width = idx;
    while (file.data[idx] == '\n')
        ++idx;
    const int linewidth = idx;
    const int height = (fileSize+2) / linewidth;

    DEBUGLOG("width = %d, linewidth = %d, height = %d\n", width, linewidth, height);

    int sx = 0, sy = 0, ex = 0, ey = 0;
    while (idx < fileSize)
    {
        if (file.data[idx] == 'S')
        {
            sx = POSX(idx);
            sy = POSY(idx);
            if (sx && ex)
                break;
        }
        else if (file.data[idx] == 'E')
        {
            ex = POSX(idx);
            ey = POSY(idx);
            if (sx && ex)
                break;
        }
        ++idx;
    }

    DEBUGLOG("start = [%d,%d], end = [%d,%d]\n", sx, sy, ex, ey);
    fill(file.data, linewidth, sx, sy, RIGHT, 0);

    uint64_t sum1 = best;

    print_uint64(sum1);

    uint64_t sum2 = 0;

    for (int y = 0; y < height; ++y)
        sum2 += _mm_popcnt_u64(gridfast[y][0]) + _mm_popcnt_u64(gridfast[y][1]) + _mm_popcnt_u64(gridfast[y][2]);

    print_uint64(sum2);

#if defined(ENABLE_DEBUGLOG) && defined(SPAM)
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (gridfast[y][x >> 6] & ((uint64_t)1 << (x & 0x3F)))
                file.data[IDXOF(x,y)] = 'O';
        }
    }
    DEBUGLOG("%.*s\n", linewidth*height, file.data);
#endif

    return 0;
}
