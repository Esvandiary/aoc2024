#define ENABLE_DEBUGLOG
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <stdbool.h>

#define WIDTH 5
#define HEIGHT 7
#define STRIDE (WIDTH+1)

static bool fits(const char* lock, const char* key)
{
    for (uint32_t y = 1; y < HEIGHT - 1; ++y)
        for (uint32_t x = 0; x < WIDTH; ++x)
            if (lock[y*STRIDE + x] == '#' && key[y*STRIDE +x] == '#')
                return false;
    return true;
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    uint32_t keys[512];
    uint32_t locks[512];
    uint32_t keyscount = 0;
    uint32_t lockscount = 0;

    uint32_t idx = 0;
    while (idx < fileSize - 2)
    {
        if (file.data[idx] == '#')
            locks[lockscount++] = idx;
        else if (file.data[idx] == '.')
            keys[keyscount++] = idx;
        idx += STRIDE*HEIGHT + 1; // (5+'\n')x7 + '\n'
    }

    DEBUGLOG("%u locks, %u keys\n", lockscount, keyscount);

    uint64_t sum1 = 0;

    for (uint32_t l = 0; l < lockscount; ++l)
    {
        for (uint32_t k = 0; k < keyscount; ++k)
        {
            if (fits(file.data + locks[l], file.data + keys[k]))
                ++sum1;
        }
    }

    print_uint64(sum1);

    return 0;
}
