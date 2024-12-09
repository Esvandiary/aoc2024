#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <stdbool.h>

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define PART1 (1 << 0)
#define PART2 (1 << 1)

static int64_t numbers[64];
static int64_t nummuls[64];

static uint8_t is_possible(int count, int64_t target, int n, int64_t running_total, uint8_t success)
{
    if (n == count)
        return (running_total == target) ? success : 0;
    if (running_total > target)
        return false;
    
    return is_possible(count, target, n + 1, running_total + numbers[n], success)
        | is_possible(count, target, n + 1, running_total * numbers[n], success)
        | is_possible(count, target, n + 1, running_total * nummuls[n] + numbers[n], PART2);
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int idx = 0;

    uint64_t sum1 = 0, sum2 = 0;

    while (idx < fileSize - 2)
    {
        int count = 0;

        int64_t num0 = 0;
        while (isdigit(file.data[idx]))
            num0 = (num0 * 10) + (file.data[idx++] & 0xF);
        ++idx; // ':'
        while (file.data[idx] != '\n')
        {
            ++idx; // ' '
            numbers[count] = 0;
            nummuls[count] = 1;
            while (isdigit(file.data[idx]))
            {
                numbers[count] = (numbers[count] * 10) + (file.data[idx++] & 0xF);
                nummuls[count] *= 10;
            }
            ++count;
        }
        ++idx; // '\n'

        uint8_t result = is_possible(count, num0, 1, numbers[0], PART1);
        if ((result & PART1) == PART1)
            sum1 += num0;
        if (result)
            sum2 += num0;
    }

    print_uint64(sum1);
    print_uint64(sum2);

    return 0;
}
