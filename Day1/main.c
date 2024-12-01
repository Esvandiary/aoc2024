#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"
#include "../common/radixsort.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    //
    // content
    //

    uint64_t list1[2048];
    uint64_t list2[2048];
    size_t count = 0;

    int idx = 0;
    do
    {
        uint32_t num1 = 0, num2 = 0;

        while (isdigit(file.data[idx]))
            num1 = (num1 * 10) + (file.data[idx++] & 0xF);
        while (file.data[idx] == ' ')
            ++idx;
        while (isdigit(file.data[idx]))
            num2 = (num2 * 10) + (file.data[idx++] & 0xF);
        ++idx; // newline

        list1[count] = num1;
        list2[count] = num2;
        ++count;
    } while (idx < fileSize - 2);

    uint64_t buf[2048];
    radixSort(list1, count, buf);
    radixSort(list2, count, buf);

    uint64_t sum1 = 0, sum2 = 0;

    for (size_t n = 0; n < count; ++n)
        sum1 += abs((int64_t)list1[n] - (int64_t)list2[n]);

    print_uint64(sum1);

    uint64_t last_num = UINT64_MAX;
    uint64_t last_result = 0;
    for (size_t l1idx = 0, l2idx = 0; l1idx < count; ++l1idx)
    {
        if (list1[l1idx] == last_num)
        {
            sum2 += last_result;
            continue;
        }

        while (list1[l1idx] > list2[l2idx])
        {
            ++l2idx;
            if (l2idx == count)
                goto p2end;
        }

        uint64_t mcount = 0;
        while (list1[l1idx] == list2[l2idx])
        {
            ++mcount;
            ++l2idx;
            if (l2idx == count)
                break;
        }
        last_result = (list1[l1idx] * mcount);
        last_num = list1[l1idx];
        sum2 += last_result;
    }
p2end:
    print_uint64(sum2);

    return 0;
}
