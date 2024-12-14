#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <stdbool.h>

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define width 101
#define height 103

typedef struct robot
{
    int64_t px;
    int64_t py;
    int64_t vx;
    int64_t vy;
} robot;

static robot robots[1024];
static uint64_t robotcount;

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    const int64_t p1iters = 100;

    uint8_t grid[width*height] = {0};

    uint64_t sum1 = 0, sum2 = 0;

    int64_t quadrants[4] = {0};

    int idx = 0;
    while (idx < fileSize - 2)
    {
        int64_t px = 0, py = 0, vx = 0, vy = 0;
        bool vxneg = 0, vyneg = 0;

        idx += 2; // 'p='
        while (isdigit(file.data[idx]))
            px = (px * 10) + (file.data[idx++] & 0xF);
        idx += 1; // ','
        while (isdigit(file.data[idx]))
            py = (py * 10) + (file.data[idx++] & 0xF);
        idx += 3; // ' v='
        if (file.data[idx] == '-')
        {
            vxneg = true;
            ++idx;
        }
        while (isdigit(file.data[idx]))
            vx = (vx * 10) + (file.data[idx++] & 0xF);
        idx += 1; // ','
        if (file.data[idx] == '-')
        {
            vyneg = true;
            ++idx;
        }
        while (idx < fileSize && isdigit(file.data[idx]))
            vy = (vy * 10) + (file.data[idx++] & 0xF);
        idx += 1; // '\\n'

        if (vxneg) vx = width - vx;
        if (vyneg) vy = height - vy;

        robots[robotcount++] = (robot) { px, py, vx, vy };

        int64_t px100 = (px + (vx * p1iters)) % width;
        int64_t py100 = (py + (vy * p1iters)) % height;

        if (px100 != width/2 && py100 != height/2)
        {
            ++quadrants[(2*py100/height)*2 | 2*px100/width];
#if defined(ENABLE_DEBUGLOG)
            grid[py100*width+px100] += 1;
            DEBUGLOG("[%" PRId64 ",%" PRId64 "] --> [%" PRId64 ",%" PRId64 "], quadrant %" PRId64 "\n", px, py, px100, py100, (2*py100/height)*2 | 2*px100/width);
#endif
        }
        else
        {
            DEBUGLOG("[%" PRId64 ",%" PRId64 "] --> [%" PRId64 ",%" PRId64 "], middle\n", px, py, px100, py100);
        }
    }

#if defined(ENABLE_DEBUGLOG) && defined(SPAM)
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            uint8_t v = grid[y*width+x];
            DEBUGLOG("%c", v > 0 ? v + 48 : '.');
        }
        DEBUGLOG("\n");
    }
#endif
    DEBUGLOG("P1 quadrants: [%" PRId64 ", %" PRId64 ", %" PRId64 ", %" PRId64 "]\n", quadrants[0], quadrants[1], quadrants[2], quadrants[3]);

    sum1 = (quadrants[0] * quadrants[1] * quadrants[2] * quadrants[3]);

    print_uint64(sum1);

    int64_t hcandidate = -1;
    int64_t vcandidate = -1;
    uint8_t xcount[width] = {0};
    uint8_t ycount[height] = {0};
    while (sum2 < 10000 && (hcandidate < 0 || vcandidate < 0))
    {
        ++sum2;

        memset(xcount, 0, sizeof(xcount));
        memset(ycount, 0, sizeof(ycount));

        for (int i = 0; i < robotcount; ++i)
        {
            robots[i].px = (robots[i].px + robots[i].vx) % width;
            robots[i].py = (robots[i].py + robots[i].vy) % height;

            ++xcount[robots[i].px];
            ++ycount[robots[i].py];
        }

        int xover30 = 0, yover30 = 0;
        for (int x = 0; x < width; ++x)
        {
            if (xcount[x] >= 30)
                ++xover30;
        }
        for (int y = 0; y < height; ++y)
        {
            if (ycount[y] >= 30)
                ++yover30;
        }

        if (xover30 >= 2)
        {
            DEBUGLOG("V candidate: %" PRIu64 "\n", sum2);
            vcandidate = sum2;
        }
        if (yover30 >= 2)
        {
            DEBUGLOG("H candidate: %" PRIu64 "\n", sum2);
            hcandidate = sum2;
        }

#if defined(ENABLE_DEBUGLOG) && defined(SPAM)
        DEBUGLOG("[%" PRIu64 "]\n", sum2);
        memset(grid, 0, width*height*sizeof(uint8_t));
        for (int i = 0; i < robotcount; ++i)
            ++grid[robots[i].py * width + robots[i].px];
        int ycnt[101] = {0};
        for (int y = 0; y < height; ++y)
        {
            int xcnt = 0;
            for (int x = 0; x < width; ++x)
            {
                uint8_t v = grid[y*width+x];
                xcnt += v;
                ycnt[x] += v;
                DEBUGLOG("%c", v > 0 ? v + 48 : '.');
            }
            DEBUGLOG("  %d\n", xcnt);
        }
        DEBUGLOG("\n\n");
        for (int x = 0; x < width; ++x)
            DEBUGLOG("%c", ycnt[x] >= 10 ? (ycnt[x]/10) + 48 : ' ');
        DEBUGLOG("\n");
        for (int x = 0; x < width; ++x)
            DEBUGLOG("%c", (ycnt[x] % 10) + 48);
        DEBUGLOG("\n\n\n");
#endif
    }

    if (hcandidate >= 0 && vcandidate >= 0)
    {
        do
        {
            if (hcandidate < vcandidate)
                hcandidate += height;
            else
                vcandidate += width;
        } while (hcandidate != vcandidate);

        sum2 = vcandidate;
        print_uint64(sum2);
    }
    else
    {
        printf("FAIL\n");
    }

    return 0;
}
