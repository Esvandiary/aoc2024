#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define FREE -1

static int16_t nums[262144];

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int idx = 0;
    int numpos = 0;
    int16_t file_id = 0;
    while (idx < fileSize && isdigit(file.data[idx]))
    {
        int file_len = file.data[idx++] - 48;
        int free_len = file.data[idx++] - 48;

        for (int i = 0; i < file_len; ++i)
            nums[numpos++] = file_id;
        for (int i = 0; i < free_len; ++i)
            nums[numpos++] = FREE;
        
        ++file_id;
    }

    int lastfile = numpos-1;
    while (nums[lastfile] == FREE)
        --lastfile;

    int firstfree = 0;
    while (nums[firstfree] != FREE)
        ++firstfree;
    
    while (firstfree < lastfile)
    {
        DEBUGLOG("firstfree = %d, lastfile = %d\n", firstfree, lastfile);

        nums[firstfree++] = nums[lastfile];
        nums[lastfile--] = FREE;

        while (nums[lastfile] == FREE)
            --lastfile;
        while (nums[firstfree] != FREE)
            ++firstfree;
    }

    uint64_t sum1 = 0;

    for (int i = 0; i < numpos; ++i)
    {
        if (nums[i] != FREE)
            sum1 += (i * nums[i]);
    }

#if defined(ENABLE_DEBUGLOG)
    for (int i = 0; i < numpos; ++i)
    {
        if (nums[i] != FREE)
            DEBUGLOG("[%d]", nums[i]);
        else
            DEBUGLOG(".");
    }
    DEBUGLOG("\n");
#endif

    print_uint64(sum1);

    return 0;
}
