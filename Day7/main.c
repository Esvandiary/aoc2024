#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <stdbool.h>

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define PART1 (1 << 0)
#define PART2 (1 << 1)

static int64_t numbers[64];
static int nummuls[64];

static bool is_possible_p1(int count, int64_t target, int n, int64_t running_total)
{
    if (n == count)
        return (running_total == target);
    if (running_total > target)
        return false;
    
    return is_possible_p1(count, target, n + 1, running_total + numbers[n])
        || is_possible_p1(count, target, n + 1, running_total * numbers[n]);
}

static bool is_possible_p2(int count, int64_t target, int n, int64_t running_total)
{
    if (n == count)
        return (running_total == target);
    if (running_total > target)
        return false;
    return is_possible_p2(count, target, n + 1, running_total + numbers[n])
        || is_possible_p2(count, target, n + 1, running_total * numbers[n])
        || is_possible_p2(count, target, n + 1, running_total * nummuls[n] + numbers[n]);
}

static bool is_possible_reverse_p1(int count, int64_t running_total, int n)
{
    if (n == 0)
        return (running_total == numbers[0]);

    if (running_total >= numbers[n] && is_possible_reverse_p1(count, running_total - numbers[n], n-1))
        return true;
    if ((running_total % numbers[n]) == 0 && is_possible_reverse_p1(count, running_total / numbers[n], n-1))
        return true;
    return false;
}

static bool is_possible_reverse_p2(int count, int64_t running_total, int n)
{
    if (n == 0)
        return (running_total == numbers[0]);

    if (running_total >= numbers[n] && is_possible_reverse_p2(count, running_total - numbers[n], n-1))
        return true;
    if ((running_total % numbers[n]) == 0 && is_possible_reverse_p2(count, running_total / numbers[n], n-1))
        return true;
    if ((running_total % nummuls[n]) == numbers[n] && is_possible_reverse_p2(count, running_total / nummuls[n], n-1))
        return true;
    return false;
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
        while (idx < fileSize && file.data[idx] != '\n')
        {
            ++idx; // ' '
            numbers[count] = 0;
            nummuls[count] = 1;
            while (idx < fileSize && isdigit(file.data[idx]))
            {
                numbers[count] = (numbers[count] * 10) + (file.data[idx++] & 0xF);
                nummuls[count] *= 10;
            }
            ++count;
        }
        while (idx < fileSize && !isdigit(file.data[idx]))
            ++idx;

        if (is_possible_reverse_p1(count, num0, count-1))
        {
            sum1 += num0;
            sum2 += num0;
        }
        else if (is_possible_reverse_p2(count, num0, count-1))
        {
            sum2 += num0;
        }
    }

    print_uint64(sum1);
    print_uint64(sum2);

    return 0;
}
