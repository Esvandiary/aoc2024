#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#if !defined(min)
    #define min(x,y) (((x) < (y)) ? (x) : (y))
#endif

static int16_t nums1[262144];

typedef struct span
{
    int16_t file_id;
    int8_t len;
    int8_t idx;
} span;

static span nums2[262144];

#define FREE -1

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int idx = 0;
    int16_t file_id = 0;
    int numpos = 0;
    while (idx < fileSize && isdigit(file.data[idx]))
    {
        int file_len = file.data[idx++] - 48;
        int free_len = file.data[idx++] - 48;

        for (int i = 0; i < file_len; ++i)
        {
            nums1[numpos] = file_id;
            nums2[numpos] = (struct span) { file_id, file_len, i };
            ++numpos;
        }
        for (int i = 0; i < free_len; ++i)
        {
            nums1[numpos] = FREE;
            nums2[numpos] = (struct span) { FREE, free_len, i };
            ++numpos;
        }

        ++file_id;
    }

    int lastfile = numpos-1;
    while (nums1[lastfile] == FREE)
        --lastfile;
    int firstfree = 0;
    while (nums1[firstfree] != FREE)
        ++firstfree;
    
    int p1firstfree = firstfree, p1lastfile = lastfile;
    while (p1firstfree < p1lastfile)
    {
        DEBUGLOG("[P1] firstfree = %d, lastfile = %d\n", p1firstfree, p1lastfile);
        nums1[p1firstfree++] = nums1[p1lastfile];
        nums1[p1lastfile--] = FREE;
        while (nums1[p1lastfile] == FREE)
            --p1lastfile;
        while (nums1[p1firstfree] != FREE)
            ++p1firstfree;
    }
    uint64_t sum1 = 0;
    for (int i = 0; i < numpos; ++i)
    {
        if (nums1[i] == FREE)
            break;
        sum1 += (i * nums1[i]);
    }

#if defined(ENABLE_DEBUGLOG)
    for (int i = 0; i < numpos; ++i)
    {
        if (nums1[i] != FREE)
            DEBUGLOG("[%d]", nums1[i]);
        else
            DEBUGLOG(".");
    }
    DEBUGLOG("\n");
#endif

    print_uint64(sum1);

    int p2firstfree = firstfree, p2lastfile = lastfile;

    int p2furthestfile = 0;
    while (p2firstfree < p2lastfile)
    {
        const uint8_t filelen = nums2[p2lastfile].len;

        int thisfree = p2firstfree;
        while (nums2[thisfree].file_id != FREE || nums2[thisfree].len < filelen)
        {
            thisfree += nums2[thisfree].len;
            if (thisfree >= p2lastfile)
            {
                if (!p2furthestfile)
                    p2furthestfile = p2lastfile;
                goto nextiter;
            }
        }

        const uint8_t freelen = nums2[thisfree].len;
        memcpy(nums2 + thisfree, nums2 + p2lastfile - nums2[p2lastfile].idx, nums2[p2lastfile].len * sizeof(span));

        for (int i = 0; i < filelen; ++i)
            nums2[p2lastfile + i - nums2[p2lastfile].idx].file_id = FREE;
        
        for (int i = filelen; i < freelen; ++i)
        {
            nums2[thisfree + i].idx -= filelen;
            nums2[thisfree + i].len -= filelen;
            DEBUGLOG("updated free @ %d --> idx %d len %d\n", thisfree + i, nums2[thisfree + i].idx, nums2[thisfree + i].len);
        }

        if (thisfree == p2firstfree)
            p2firstfree += filelen;

        DEBUGLOG("wrote file_id %d len %d, firstfree now %d\n", nums2[thisfree].file_id, filelen, p2firstfree);

    nextiter:
        p2lastfile -= filelen;
        while (p2firstfree < p2lastfile && nums2[p2firstfree].file_id != FREE)
        {
            DEBUGLOG("firstfree @ %d file id = %d\n", p2firstfree, nums2[p2firstfree].file_id);
            p2firstfree += nums2[p2firstfree].len - nums2[p2firstfree].idx;
            DEBUGLOG("updated firstfree --> %d\n", p2firstfree);
        }
        while (p2firstfree < p2lastfile && nums2[p2lastfile].file_id == FREE)
            p2lastfile -= nums2[p2lastfile].idx + 1;
    }

#if defined(ENABLE_DEBUGLOG)
    DEBUGLOG("P2 ");
    for (int i = 0; i < p2furthestfile + 10; ++i)
    {
        if (nums2[i].file_id != FREE)
            DEBUGLOG("[%d]", nums2[i].file_id);
        else
            DEBUGLOG(".");
    }
    DEBUGLOG("\n");
#endif

    uint64_t sum2 = 0;

    int p2idx = 0;
    while (p2idx < p2furthestfile + 10)
    {
        if (nums2[p2idx].file_id != FREE)
        {
            for (int i = 0; i < nums2[p2idx].len; ++i)
                sum2 += ((p2idx+i) * nums2[p2idx].file_id);
        }
        p2idx += nums2[p2idx].len;
    }

    print_uint64(sum2);

    return 0;
}
