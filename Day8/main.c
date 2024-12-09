#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <nmmintrin.h>
#include <stdbool.h>

uint64_t antinodes1[64];
uint64_t antinodes2[64];

typedef struct pos
{
    int16_t x;
    int16_t y;
} pos;

pos indexes[80][16];
int indexcounts[80];

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int idx = 0;
    while (file.data[idx] != '\n')
        ++idx;
    const int width = idx;
    const int linewidth = ++idx;
    const int height = (fileSize + 2) / linewidth;

    idx = 0;
    while (idx < fileSize)
    {
        const char c = file.data[idx];
        if (c >= 48)
        {
            indexes[c-48][indexcounts[c-48]].x = (idx % linewidth);
            indexes[c-48][indexcounts[c-48]].y = (idx / linewidth);
            ++indexcounts[c-48];
        }
        ++idx;
    }

    for (int c = 0; c < 80; ++c)
    {
        const int count = indexcounts[c];
        if (count >= 2)
        {
            for (int i = 0; i < count-1; ++i)
            {
                antinodes2[indexes[c][i].y] |= (uint64_t)1 << indexes[c][i].x;

                for (int j = i+1; j < count; ++j)
                {
                    int xdiff = indexes[c][i].x - indexes[c][j].x;
                    int ydiff = indexes[c][i].y - indexes[c][j].y;

                    pos new1, new2;
                    new1.x = indexes[c][i].x + xdiff;
                    new1.y = indexes[c][i].y + ydiff;
                    new2.x = indexes[c][j].x - xdiff;
                    new2.y = indexes[c][j].y - ydiff;

                    if (new1.x >= 0 && new1.x < width && new1.y >= 0 && new1.y < height)
                    {
                        antinodes1[new1.y] |= (uint64_t)1 << new1.x;
                        do
                        {
                            antinodes2[new1.y] |= (uint64_t)1 << new1.x;
                            new1.x += xdiff;
                            new1.y += ydiff;
                        } while (new1.x >= 0 && new1.x < width && new1.y >= 0 && new1.y < height);
                    }
                    if (new2.x >= 0 && new2.x < width && new2.y >= 0 && new2.y < height)
                    {
                        antinodes1[new2.y] |= (uint64_t)1 << new2.x;
                        do
                        {
                            antinodes2[new2.y] |= (uint64_t)1 << new2.x;
                            new2.x -= xdiff;
                            new2.y -= ydiff;
                        } while (new2.x >= 0 && new2.x < width && new2.y >= 0 && new2.y < height);
                    }
                }
            }

            antinodes2[indexes[c][count-1].y] |= (uint64_t)1 << indexes[c][count-1].x;
        }
    }

    uint64_t sum1 = 0, sum2 = 0;

    for (int y = 0; y < height; ++y)
        sum1 += _mm_popcnt_u64(antinodes1[y]);
    for (int y = 0; y < height; ++y)
        sum2 += _mm_popcnt_u64(antinodes2[y]);

    print_uint64(sum1);
    print_uint64(sum2);

#if defined(ENABLE_DEBUGLOG)
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            printf("%c", ((antinodes1[y] >> x) & 1) ? 'X' : '.');
        }
        printf("\n");
    }
#endif

    return 0;
}
