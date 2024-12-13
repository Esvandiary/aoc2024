#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <stdbool.h>
#include <math.h>

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#if !defined(max)
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#define P2OFFSET 10000000000000LL

void solve(double a, double b, double c, double d, double u, double v, double* x, double* y)
{
    if (a > c)
    {
        double f = u * c / a;
        double g = b * c / a;
        *y = (v - f) / (d - g);
        if (c != 0)
            *x = (f - g * (*y)) / c;
        else
            *x = (u - b * (*y)) / a;
    }
    else
    {
        double f = v * a / c;
        double g = d * a / c;
        *y = (u - f) / (b - g);
        if (a != 0)
            *x = (f - g * (*y)) / a;
        else
            *x = (v - d * (*y)) / c;
    }
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    uint64_t sum1 = 0, sum2 = 0;

    int idx = 0;
    while (idx < fileSize - 2)
    {
        idx += 12; // 'Button A: X+'
        int32_t ax = (file.data[idx++] & 0xF)*10 + (file.data[idx++] & 0xF);
        idx += 4; // ', Y+'
        int32_t ay = (file.data[idx++] & 0xF)*10 + (file.data[idx++] & 0xF);
        idx += 13; // '\nButton B: X+'
        int32_t bx = (file.data[idx++] & 0xF)*10 + (file.data[idx++] & 0xF);
        idx += 4; // ', Y+'
        int32_t by = (file.data[idx++] & 0xF)*10 + (file.data[idx++] & 0xF);
        idx += 10; // '\nPrize: X='
        int64_t xresult = 0;
        while (idx < fileSize && isdigit(file.data[idx]))
            xresult = (xresult * 10) + (file.data[idx++] & 0xF);
        idx += 4; // ', Y='
        int64_t yresult = 0;
        while (idx < fileSize && isdigit(file.data[idx]))
            yresult = (yresult * 10) + (file.data[idx++] & 0xF);
        idx += 2; // '\n\n'

        double amul = 0, bmul = 0;
        solve(ax, bx, ay, by, xresult, yresult, &amul, &bmul);

        int64_t aimul, bimul;

        aimul = round(amul);
        bimul = round(bmul);
        if (aimul < 100 && bimul < 100 && ax * aimul + bx * bimul == xresult && ay * aimul + by * bimul == yresult)
        {
            DEBUGLOG("P1: A [%d,%d] B [%d,%d] aim [%" PRId64 ",%" PRId64 "], amul %.2f, bmul %.2f\n",
                ax, ay, bx, by, xresult, yresult, amul, bmul);

            sum1 += (aimul*3) + bimul;
        }

        solve(ax, bx, ay, by, xresult + P2OFFSET, yresult + P2OFFSET, &amul, &bmul);

        aimul = round(amul);
        bimul = round(bmul);
        if (ax * aimul + bx * bimul == xresult + P2OFFSET && ay * aimul + by * bimul == yresult + P2OFFSET)
        {
            DEBUGLOG("P2: A [%d,%d] B [%d,%d] aim [%" PRId64 ",%" PRId64 "], amul %.2f, bmul %.2f\n",
                ax, ay, bx, by, (int64_t)(xresult + P2OFFSET), (int64_t)(yresult + P2OFFSET), amul, bmul);

            sum2 += (aimul*3) + bimul;
        }
    }

    print_int64(sum1);
    print_int64(sum2);

    return 0;
}
