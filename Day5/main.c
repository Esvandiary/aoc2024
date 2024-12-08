#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <stdbool.h>

#define isdigit(c) ((c) >= '0' && (c) <= '9')

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    uint64_t sum1 = 0, sum2 = 0;

    int revlookup[100][100] = {0};
    int revlookupcnt[100] = {0};

    int idx = 0;
    while (file.data[idx] != '\n')
    {
        long num1 = 0, num2 = 0;
        while (isdigit(file.data[idx]))
            num1 = (num1 * 10) + (file.data[idx++] & 0xF);
        ++idx; // |
        while (isdigit(file.data[idx]))
            num2 = (num2 * 10) + (file.data[idx++] & 0xF);
        ++idx; // \n

        revlookup[num2][revlookupcnt[num2]++] = num1;
    }
    ++idx; // \n

    int pages[64];
    int count;

    while (idx < fileSize - 2)
    {
        count = 0;
        while (file.data[idx] != '\n')
        {
            pages[count] = 0;
            while (isdigit(file.data[idx]))
                pages[count] = (pages[count] * 10) + (file.data[idx++] & 0xF);
            ++count;

            if (file.data[idx] == ',')
                ++idx;
        }
        ++idx; // \n

        bool pristine = true;
        bool solved;
        do
        {
            solved = true;
            for (int i = 0; i < count - 1; ++i)
            {
                for (int j = i + 1; j < count; ++j)
                {
                    for (int k = 0; k < revlookupcnt[pages[i]]; ++k)
                    {
                        if (pages[j] == revlookup[pages[i]][k])
                        {
                            pristine = false;
                            solved = false;
                            int tmp = pages[j];
                            pages[j] = pages[i];
                            pages[i] = tmp;
                        }
                    }
                }
            }
        } while (!solved);

        if (pristine)
            sum1 += pages[count/2];
        else
            sum2 += pages[count/2];
    }

    print_uint64(sum1);
    print_uint64(sum2);

    return 0;
}