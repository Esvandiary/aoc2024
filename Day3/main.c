#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    uint64_t sum1 = 0, sum2 = 0;

    int idx = 0;
    int p2mul = 1;
    while (idx < fileSize)
    {
        if (file.data[idx] == 'm')
        {
            if (idx + 7 >= fileSize) // mul(1,2)
                break;
            if (file.data[++idx] != 'u')
                continue;
            if (file.data[++idx] != 'l')
                continue;
            if (file.data[++idx] != '(')
                continue;
            ++idx;
            if (!isdigit(file.data[idx]))
                continue;
            long num1 = 0, num2 = 0;
            while (idx < fileSize && isdigit(file.data[idx]))
                num1 = (num1 * 10) + (file.data[idx++] & 0xF);
            if (idx >= fileSize || file.data[idx++] != ',')
                continue;
            while (idx < fileSize && isdigit(file.data[idx]))
                num2 = (num2 * 10) + (file.data[idx++] & 0xF);
            if (idx >= fileSize || file.data[idx++] != ')')
                continue;

            const long n = (num1 * num2);
            sum1 += n;
            sum2 += n * p2mul;
        }
        else if (file.data[idx] == 'd')
        {
            if (idx + 7 >= fileSize) // don't()
                break;
            if (file.data[++idx] != 'o')
                continue;
            if (file.data[idx+1] == '(' && file.data[idx+2] == ')')
            {
                p2mul = 1;
                idx += 3;
                continue;
            }
            else if (file.data[idx+1] == 'n' && file.data[idx+2] == '\'' && file.data[idx+3] == 't' && file.data[idx+4] == '(' && file.data[idx+5] == ')')
            {
                p2mul = 0;
                idx += 6;
                continue;
            }
            else
            {
                ++idx;
                continue;
            }
        }
        else
        {
            ++idx;
        }
    }

    print_uint64(sum1);
    print_uint64(sum2);

    return 0;
}
