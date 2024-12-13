// #define ENABLE_DEBUGLOG
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <stdbool.h>

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#if !defined(max)
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    uint64_t sum1 = 0, sum2 = 0;

    int idx = 0;
    while (idx < fileSize - 2)
    {
        idx += 12; // 'Button A: X+'
        uint32_t ax = (file.data[idx++] & 0xF)*10 + (file.data[idx++] & 0xF);
        idx += 4; // ', Y+'
        uint32_t ay = (file.data[idx++] & 0xF)*10 + (file.data[idx++] & 0xF);
        idx += 13; // '\nButton B: X+'
        uint32_t bx = (file.data[idx++] & 0xF)*10 + (file.data[idx++] & 0xF);
        idx += 4; // ', Y+'
        uint32_t by = (file.data[idx++] & 0xF)*10 + (file.data[idx++] & 0xF);
        idx += 10; // '\nPrize: X='
        uint32_t xresult = 0;
        while (idx < fileSize && isdigit(file.data[idx]))
            xresult = (xresult * 10) + (file.data[idx++] & 0xF);
        idx += 4; // ', Y='
        uint32_t yresult = 0;
        while (idx < fileSize && isdigit(file.data[idx]))
            yresult = (yresult * 10) + (file.data[idx++] & 0xF);
        idx += 2; // '\n\n'

        uint32_t xmax = max(ax, bx), ymax = max(ay, by);
        float xmul = (float)(xresult) / xmax, ymul = (float)(yresult) / ymax;

        bool afirst = (xmul > (ymul * 3)) ? (xmax == ax) : (ymax == ay);
        bool xprimary = (xmul <= ymul);

        DEBUGLOG("A [%u,%u] B [%u,%u] aim [%u,%u] max [%u,%u] mul [%.2f,%.2f], i1=%c%c, i2=%c%c, j1=%c%c, j2=%c%c\n",
            ax, ay, bx, by, xresult, yresult, xmax, ymax, xmul, ymul,
            afirst ? 'a' : 'b', xprimary ? 'x' : 'y', afirst ? 'a' : 'b', xprimary ? 'y' : 'x',
            afirst ? 'b' : 'a', xprimary ? 'x' : 'y', afirst ? 'b' : 'a', xprimary ? 'y' : 'x');

        uint32_t i1 = (afirst ? (xprimary ? ax : ay) : (xprimary ? bx : by));
        uint32_t i2 = (afirst ? (xprimary ? ay : ax) : (xprimary ? by : bx));
        uint32_t j1 = (afirst ? (xprimary ? bx : by) : (xprimary ? ax : ay));
        uint32_t j2 = (afirst ? (xprimary ? by : bx) : (xprimary ? ay : ax));
        uint32_t n1result = (xprimary ? xresult : yresult);
        uint32_t n2result = (xprimary ? yresult : xresult);

        int32_t i1cur = 0, i2cur = 0;
        for (int i = 0; i < 100 && i1cur < n1result; ++i)
        {
            // DEBUGLOG("[%d] i1 = %d, i2 = %d\n", i, i1cur, i2cur);
            if (((n2result - i2cur) % j2) == 0)
            {
                int32_t c2mul = (n2result - i2cur) / j2;
                DEBUGLOG("[%d] n2 match with mul %d\n", i, c2mul);
                if (i1cur + (c2mul * j1) == n1result)
                {
                    DEBUGLOG("n1 match\n");
                    sum1 += (i1cur / i1)*(afirst ? 3 : 1) + c2mul*(afirst ? 1 : 3);
                    break;
                }
            }
            i1cur += i1;
            i2cur += i2;
        }
    }

    print_uint64(sum1);

    return 0;
}
