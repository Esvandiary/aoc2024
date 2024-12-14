#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <stdbool.h>

#define isdigit(c) ((c) >= '0' && (c) <= '9')

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

    const int64_t width = 101;
    const int64_t height = 103;
    const int64_t p1iters = 100;

    uint8_t* grid = (uint8_t*)calloc(width*height, sizeof(uint8_t));

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

#if defined(ENABLE_DEBUGLOG)
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            uint8_t v = grid[y*width+x];
            DEBUGLOG("%c", v > 0 ? v + 48 : '.');
        }
        DEBUGLOG("\n");
    }
    DEBUGLOG("P1 quadrants: [%" PRId64 ", %" PRId64 ", %" PRId64 ", %" PRId64 "]\n", quadrants[0], quadrants[1], quadrants[2], quadrants[3]);
#endif

    sum1 = (quadrants[0] * quadrants[1] * quadrants[2] * quadrants[3]);

    print_uint64(sum1);

    int64_t p2iters = 0;
    int64_t hcandidate = -1;
    int64_t vcandidate = -1;
    while (++p2iters < 10000 && (hcandidate < 0 || vcandidate < 0))
    {
        ++sum2;

        int hsegs[3] = {0};
        int vsegs[3] = {0};

        for (int i = 0; i < robotcount; ++i)
        {
            robots[i].px = (robots[i].px + robots[i].vx) % width;
            robots[i].py = (robots[i].py + robots[i].vy) % height;

            ++hsegs[3 * robots[i].px / width];
            ++vsegs[3 * robots[i].py / height];
        }

        if (hsegs[1] >= hsegs[0] * 3 / 2 && hsegs[1] >= hsegs[2] * 3 / 2)
        {
            DEBUGLOG("H candidate: %" PRIu64 "\n", sum2);
            hcandidate = sum2;
        }
        if (vsegs[1] >= vsegs[0] * 3 / 2 && vsegs[1] >= vsegs[2] * 3 / 2)
        {
            DEBUGLOG("V candidate: %" PRIu64 "\n", sum2);
            vcandidate = sum2;
        }
    }

    if (hcandidate >= 0 && vcandidate >= 0)
    {
        do
        {
            if (hcandidate < vcandidate)
                hcandidate += width;
            else
                vcandidate += height;
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
