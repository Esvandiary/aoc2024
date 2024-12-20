// #define ENABLE_DEBUGLOG
// #define SPAM
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"
#include "../common/uthash/uthash.h"

#include <stdbool.h>

static inline FORCEINLINE const char* matchstart(const char* str, int len, const char* match)
{
    while (len != 0 && *str != '\0' && *match != '\0' && *str == *match)
    {
        ++str;
        ++match;
        --len;
    }
    return (*match == '\0') ? str : NULL;
}

static char patterns[1024][16];
static uint32_t patternscount;

typedef struct hashnode
{
    uint64_t count;
    UT_hash_handle hh;
} hashnode;

static hashnode hashnodes[262144];
static uint32_t hashnodescount;

static hashnode* hash;

static uint64_t findmatches(const char* data, int len)
{
    hashnode* hcheck;
    HASH_FIND(hh, hash, data, len, hcheck);
    if (hcheck)
        return hcheck->count;

    uint64_t count = 0;
    if (len > 0)
    {
        for (uint32_t p = 0; p < patternscount; ++p)
        {
            const char* nextstr;
            if ((nextstr = matchstart(data, len, patterns[p])) != NULL)
            {
                if (nextstr == data + len)
                    ++count;
                else
                    count += findmatches(nextstr, len - (nextstr - data));
            }
        }
    }

    hashnode* h = hashnodes + (hashnodescount++);
    h->count = count;
    HASH_ADD_KEYPTR(hh, hash, data, len, h);

    return count;
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int idx = 0;
    while (file.data[idx] != '\n')
    {
        int clen = 0;
        while (file.data[idx] != ',' && file.data[idx] != '\n')
        {
            patterns[patternscount][clen++] = file.data[idx];
            ++idx;
        }
        if (clen)
            ++patternscount;

        idx += 2; // ', ' or '\n\n'
        if (file.data[idx-1] == '\n')
            break;
    }

#if defined(ENABLE_DEBUGLOG) && defined(SPAM)
    tnode* path[12] = {0};
    for (int i = 0; i < 5; ++i)
    {
        path[0] = rootnode->next[i];
        printtowels(path, 1);
    }
#endif

    uint64_t sum1 = 0, sum2 = 0;

    while (idx < fileSize - 2)
    {
        int laidx = idx;
        while (laidx < fileSize && file.data[laidx] != '\n')
            ++laidx;
        
        DEBUGLOG("%.*s --> ", laidx - idx, file.data + idx);
        // uint32_t result = possible(0, file.data + idx, file.data + idx, laidx - idx, rootnode) ? 1 : 0;
        uint64_t result = findmatches(file.data + idx, laidx - idx);
        DEBUGLOG("%" PRIu64 "\n", result);
        sum1 += (result) ? 1 : 0;
        sum2 += result;

        idx = laidx + 1;
    }

    print_uint64(sum1);
    print_uint64(sum2);

    return 0;
}
