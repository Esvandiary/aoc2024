#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <stdbool.h>
#include <nmmintrin.h>

#define INDEXOF(x,y) ((y)*linewidth + (x))
#define CHARAT(x,y) file.data[INDEXOF((x),(y))]

#define MKDIR(dx,dy) (((((dx) + 1) & 2) | (((dx) >> 1) & 1)) | ((((dy) + 1) & 2) | (((dy) >> 1) & 1)) << 2)

#define TURN(gdx, gdy) {\
    if ((gdy) != 0) { (gdx) = -(gdy); (gdy) = 0; }\
    else { (gdy) = (gdx); (gdx) = 0; }\
}

typedef enum direction
{
    NONE = 0,
    UP    = MKDIR(0, -1),
    RIGHT = MKDIR(1, 0),
    DOWN  = MKDIR(0, 1),
    LEFT  = MKDIR(-1, 0),
} direction;

static uint64_t positions[160][3];
static uint8_t pop[160][160];
static uint8_t spop[160][160];

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int idx = 0;
    while (file.data[idx] != '\n')
        ++idx;
    const int width = idx;
    const int linewidth = ++idx;
    const int height = (fileSize+1) / linewidth;

    while (file.data[idx] != '^')
        ++idx;
    int gx = idx % linewidth, gy = idx / linewidth;
    int gdx = 0, gdy = -1;

    uint64_t sum1 = 0, sum2 = 0;

    positions[gy][gx >> 6] |= (uint64_t)1 << (gx & 0x3F);
    pop[gy][gx] |= MKDIR(gdx, gdy);

    while (LIKELY((gx + gdx) >= 0 && (gx + gdx) < width && (gy + gdy) >= 0 && (gy + gdy) < height))
    {
        while (CHARAT(gx + gdx, gy + gdy) == '#')
        {
            // change direction
            TURN(gdx, gdy);
        }

        if (pop[gy + gdy][gx + gdx] == 0)
        {
            // we didn't turn, so speculate what would happen if we did
            // we need to eventually end up running into our own path travelling in the same direction
            int sgx = gx, sgy = gy;
            int sgdx = gdx, sgdy = gdy;
            // temporarily make the blockage
            CHARAT(gx + gdx, gy + gdy) = '#';
        snextiter:
            int turns = 0;
            while (turns < 4)
            {
                if (sgx + sgdx < 0 || sgx + sgdx >= width || sgy + sgdy < 0 || sgy + sgdy >= height)
                    goto sdone;
                if (CHARAT(sgx + sgdx, sgy + sgdy) == '#')
                {
                    TURN(sgdx, sgdy);
                    ++turns;
                }
                else
                {
                    break;
                }
            }
            if (turns == 4)
            {
                DEBUGLOG("impossible??\n");
                goto sdone;
            }

            if ((pop[sgy][sgx] & MKDIR(sgdx, sgdy)) == MKDIR(sgdx, sgdy) || (spop[sgy][sgx] & MKDIR(sgdx, sgdy)) == MKDIR(sgdx, sgdy))
            {
                sum2 += 1;
                goto sdone;
            }

            spop[sgy][sgx] |= MKDIR(sgdx, sgdy);

            sgx += sgdx;
            sgy += sgdy;

            if (sgx >= 0 && sgx < width && sgy >= 0 && sgy < height)
            {
                goto snextiter;
            }
            else
            {
                goto sdone;
            }

        sdone:
            // clear the speculative blockage
            CHARAT(gx + gdx, gy + gdy) = '.';
            // clear our speculative movements
            // TODO: faster to do this or re-retrace?
            memset(spop, 0, sizeof(spop));
        }

        pop[gy][gx] |= MKDIR(gdx, gdy);

        // move
        gx += gdx;
        gy += gdy;

        positions[gy][gx >> 6] |= (uint64_t)1 << (gx & 0x3F);
    }
    // else fall out of the world

    for (int y = 0; y < height; ++y)
    {
        for (int xseg = 0; xseg < 3; ++xseg)
        {
            sum1 += _mm_popcnt_u64(positions[y][xseg]);
        }
    }

    print_uint64(sum1);
    print_uint64(sum2);

    return 0;
}
