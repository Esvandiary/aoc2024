#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <stdbool.h>
#include <stdio.h>

#define isdigit(c) ((c) >= '0' && (c) <= '9')

typedef enum direction
{
    INITIAL,
    UNKNOWN,
    DECREASING,
    INCREASING
} direction;

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    //
    // content
    //

    uint64_t sum1 = 0, sum2 = 0;

    int line = 0;
    int idx = 0;
    bool seensafe = false;
    while (idx < fileSize - 2)
    {
        if (seensafe)
        {
            while (idx < fileSize && file.data[idx] != '\n')
                ++idx;
            ++idx;
            ++sum1;
            continue;
        }

        long prevnum = 0;
        int diffs[128];
        int count = 0;
        direction dir = INITIAL;
        bool unsafe1 = 0;
        int unsafe2 = 0;
        while (idx < fileSize && file.data[idx] != '\n')
        {
            long num1 = 0;
            while (isdigit(file.data[idx]))
                num1 = (num1 * 10) + (file.data[idx++] & 0xF);
            while (idx < fileSize && file.data[idx] == ' ')
                ++idx;

            diffs[count] = num1 - prevnum;

            ++count;
            prevnum = num1;
        }
        ++idx;

#if defined(ENABLE_DEBUGLOG)
        ++line;
        DEBUGLOG("line %d | ", line);
        for (int i = 1; i < count; ++i)
            DEBUGLOG("%d ", diffs[i]);
#endif

        int incs = 0, decs = 0, eqs = 0, oors = 0;
        for (int i = 1; i < count; ++i)
        {
            if (diffs[i] > 0)
                ++incs;
            else if (diffs[i] < 0)
                ++decs;
            else
                ++eqs;

            if (diffs[i] > 3 || diffs[i] < -3)
                ++oors;
        }

        DEBUGLOG("| incs = %d, decs = %d, eqs = %d, oors = %d\n", incs, decs, eqs, oors);

        if (eqs > 1)
        {
            DEBUGLOG("unsafe eqs\n");
            continue;
        }
        if (incs > 1 && decs > 1)
        {
            DEBUGLOG("unsafe both\n");
            continue;
        }
        
        if (eqs == 0 && oors == 0 && ((incs != 0 && decs == 0) || (incs == 0 && decs != 0)))
        {
            seensafe = true;
            ++sum1;
            continue;
        }

        // try increasing
        int bads = 0;
        for (int i = 1; i < count && bads < 2; ++i)
        {
            if (diffs[i] <= 0 || diffs[i] > 3)
            {
                if (i+1 < count && diffs[i] + diffs[i+1] > 0 && diffs[i] + diffs[i+1] <= 3)
                {
                    ++bads;
                    ++i; // we've pre-checked i+1
                    continue;
                }
                else if (i > 1 && diffs[i - 1] + diffs[i] > 0 && diffs[i - 1] + diffs[i] <= 3)
                {
                    ++bads;
                    continue;
                }
                else if (i == 1 || i + 1 == count)
                {
                    ++bads;
                    continue;
                }
                else
                {
                    bads = 1000;
                    break;
                }
            }
        }
        if (bads < 2)
        {
            DEBUGLOG("pseudosafe inc\n");
            ++sum2;
            continue;
        }
        // try decreasing
        bads = 0;
        for (int i = 1; i < count && bads < 2; ++i)
        {
            if (diffs[i] >= 0 || diffs[i] < -3)
            {
                if (i+1 < count && diffs[i] + diffs[i + 1] < 0 && diffs[i] + diffs[i + 1] >= -3)
                {
                    ++bads;
                    ++i; // we've pre-checked i+1
                    continue;
                }
                else if (i > 1 && diffs[i - 1] + diffs[i] < 0 && diffs[i - 1] + diffs[i] >= -3)
                {
                    ++bads;
                    continue;
                }
                else if (i == 1 || i + 1 == count)
                {
                    ++bads;
                    continue;
                }
                else
                {
                    bads = 1000;
                    break;
                }
            }
        }
        if (bads < 2)
        {
            DEBUGLOG("pseudosafe dec\n");
            ++sum2;
            continue;
        }

        DEBUGLOG("unsafe general\n");
    }

    print_uint64(sum1);
    print_uint64(sum1 + sum2);

    return 0;
}
