#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"
#include "../common/radixsort.h"

#include <stdbool.h>

#if !defined(max)
#define max(a,b) ((a) < (b) ? (b) : (a))
#endif

#define MKIDX(c1,c2) (((uint16_t)(c1) << 5) | ((uint16_t)(c2)))
#define C1(idx) ((idx) >> 5)
#define C2(idx) ((idx) & 0x1F)

static uint16_t pairs[32*32][32];
static uint8_t pairscount[32*32];

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int idx = 0;
    while (idx < fileSize)
    {
        char c1 = file.data[idx++] - 'a';
        char c2 = file.data[idx++] - 'a';
        idx++; // '-'
        char d1 = file.data[idx++] - 'a';
        char d2 = file.data[idx++] - 'a';
        idx++; // '\n'

        pairs[MKIDX(c1,c2)][pairscount[MKIDX(c1,c2)]++] = MKIDX(d1, d2);
        pairs[MKIDX(d1,d2)][pairscount[MKIDX(d1,d2)]++] = MKIDX(c1, c2);
    }

    uint64_t sum1 = 0;

    uint32_t maxmax = 0;
    uint32_t maxmaxcount = 0;
    uint32_t maxmaxidx = 0;

    for (int c2 = 0; c2 < 26; ++c2)
    {
        uint16_t c = MKIDX(19,c2);

        DEBUGLOG("%c%c --> %u\n", 't', c2 + 'a', pairscount[c]);

        uint32_t deshared[32] = {0};

        for (int i = 0; i < pairscount[c]; ++i)
        {
            // 19,c2 - d1,d2
            uint16_t d = pairs[c][i];

            for (int j = 0; j < pairscount[c]; ++j)
            {
                if (i == j)
                    continue;

                uint16_t e = pairs[c][j];

                // check if d pairs with e
                for (int k = 0; k < pairscount[d]; ++k)
                {
                    if (pairs[d][k] == e)
                    {
                        if (i < j && (C1(d) != 19 || C2(d) > c2) && (C1(e) != 19 || C2(e) > c2))
                        {
                            // DEBUGLOG("[P1] triplet: %c%c,%c%c,%c%c\n",
                            //     't', c2 + 'a', C1(d) + 'a', C2(d) + 'a', C1(e) + 'a', C2(e) + 'a');
                            ++sum1;
                        }

                        ++deshared[i];
                        break;
                    }
                }
            }
        }

        uint32_t maxdeshared = 0;
        uint32_t maxdesharedcount = 0;
        for (int i = 0; i < 32; ++i)
        {
            if (deshared[i])
            {
                DEBUGLOG("    %c%c has %u shared links\n", C1(pairs[c][i]) + 'a', C2(pairs[c][i]) + 'a', deshared[i]);
                if (deshared[i] > maxdeshared)
                {
                    maxdeshared = deshared[i];
                    maxdesharedcount = 1;
                }
                else if (deshared[i] == maxdeshared)
                {
                    ++maxdesharedcount;
                }
            }
        }

        if (maxdeshared > maxmax)
        {
            maxmax = maxdeshared;
            maxmaxcount = maxdesharedcount;
            maxmaxidx = c;
        }
        else if (maxdeshared == maxmax && maxdesharedcount > maxmaxcount)
        {
            maxmaxcount = maxdesharedcount;
            maxmaxidx = c;
        }
    }

    print_uint64(sum1);

    DEBUGLOG("[P2] max conn = %c%c, max %u count %u\n", C1(maxmaxidx) + 'a', C2(maxmaxidx) + 'a', maxmax, maxmaxcount);

    uint16_t list[32];
    uint32_t listcount = 0;

    // add for sorting porpoises
    pairs[maxmaxidx][pairscount[maxmaxidx]++] = maxmaxidx;

    while (true)
    {
        uint16_t nextbest = UINT16_MAX;
        for (int i = 0; i < pairscount[maxmaxidx]; ++i)
        {
            // 19,c2 - d1,d2
            uint16_t d = pairs[maxmaxidx][i];

            uint32_t deshared = 0;

            for (int j = 0; j < pairscount[maxmaxidx]; ++j)
            {
                if (i == j)
                    continue;

                uint16_t e = pairs[maxmaxidx][j];

                // check if d pairs with e
                for (int k = 0; k < pairscount[d]; ++k)
                {
                    if (pairs[d][k] == e)
                    {
                        ++deshared;
                        break;
                    }
                }
            }
            
            DEBUGLOG("%c%c -> %c%c: deshared = %u, maxmax = %u, maxmaxcount = %u\n",
                C1(maxmaxidx) + 'a', C2(maxmaxidx) + 'a', C1(pairs[maxmaxidx][i]) + 'a', C2(pairs[maxmaxidx][i]) + 'a',
                deshared, maxmax, maxmaxcount);
            if ((deshared == maxmax + 1 || maxmaxidx == pairs[maxmaxidx][i])
                && pairs[maxmaxidx][i] < nextbest && (listcount == 0 || pairs[maxmaxidx][i] > list[listcount-1]))
            {
                nextbest = pairs[maxmaxidx][i];
            }
        }

        if (nextbest != UINT16_MAX)
            list[listcount++] = nextbest;
        else
            break;
    }

    printf("%c%c", C1(list[0]) + 'a', C2(list[0]) + 'a');
    for (int i = 1; i < listcount; ++i)
        printf(",%c%c", C1(list[i]) + 'a', C2(list[i]) + 'a');
    printf("\n");

    return 0;
}
