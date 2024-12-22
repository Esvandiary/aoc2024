#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <stdbool.h>

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define MIX(a, b) ((a) ^ (b))
#define PRUNE(a) ((a) & 16777215)

#define CALCDIFF(newn, oldn) (9 + (int8_t)((newn) % 10) - (int8_t)((oldn) % 10))

static inline FORCEINLINE uint64_t iterate(uint64_t n)
{
    n = PRUNE(MIX(n, (n * 64)));
    n = PRUNE(MIX(n, (n / 32)));
    n = PRUNE(MIX(n, (n * 2048)));
    return n;
}

static bool p2added[19][19][19][19];
static uint32_t p2bananas[19][19][19][19];

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    uint64_t sum1 = 0;

    int idx = 0;
    while (idx < fileSize - 2)
    {
        uint64_t num0 = 0;
        while (idx < fileSize && isdigit(file.data[idx]))
            num0 = (num0 * 10) + (file.data[idx++] & 0xF);
        ++idx; // '\n'

        memset(p2added, 0, sizeof(p2added));

        DEBUGLOG("%" PRIu64 ": ", num0);
        int8_t diff[4];
        uint64_t num1 = iterate(num0);
        diff[1] = CALCDIFF(num1, num0);
        uint64_t num2 = iterate(num1);
        diff[2] = CALCDIFF(num2, num1);
        uint64_t numN = iterate(num2);
        diff[3] = CALCDIFF(numN, num2);
        for (int i = 3; i < 2000; ++i)
        {
            uint64_t next = iterate(numN);
            diff[0] = diff[1];
            diff[1] = diff[2];
            diff[2] = diff[3];
            diff[3] = CALCDIFF(next, numN);
            if (!p2added[diff[0]][diff[1]][diff[2]][diff[3]])
            {
                p2added[diff[0]][diff[1]][diff[2]][diff[3]] = true;
                p2bananas[diff[0]][diff[1]][diff[2]][diff[3]] += (next % 10);
            }
            numN = next;
        }
        DEBUGLOG("%" PRIu64 "\n", numN);

        sum1 += numN;
    }

    print_uint64(sum1);

    int8_t bestdiff[4];
    uint32_t bestbananas = 0;
    for (int a = 0; a < 19; ++a)
    {
        for (int b = 0; b < 19; ++b)
        {
            for (int c = 0; c < 19; ++c)
            {
                for (int d = 0; d < 19; ++d)
                {
                    if (p2bananas[a][b][c][d] > bestbananas)
                    {
                        bestdiff[0] = a;
                        bestdiff[1] = b;
                        bestdiff[2] = c;
                        bestdiff[3] = d;
                        bestbananas = p2bananas[a][b][c][d];
                    }
                }
            }
        }
    }

    uint64_t sum2 = bestbananas;
    print_uint64(sum2);

    return 0;
}
