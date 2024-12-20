// #define ENABLE_DEBUGLOG
// #define SPAM
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"
#include "../common/uthash/uthash.h"

#include <stdbool.h>

typedef enum tcolor
{
    WHITE = 0,
    BLUE = 1,
    BLACK = 2,
    RED = 3,
    GREEN = 4
} tcolor;

static inline FORCEINLINE tcolor getcolor(char c)
{
    switch (c)
    {
        case 'w': return WHITE;
        case 'u': return BLUE;
        case 'b': return BLACK;
        case 'r': return RED;
        case 'g': return GREEN;
        default: return INT32_MAX; // hopefully explode
    }
}

static const char tcolornames[] = {'w', 'u', 'b', 'r', 'g'};

typedef struct node
{
    struct node* next[5];
    bool present;
} node;

static node nodestore[4096];
static uint32_t nodestorecount;

static node* rootnode;

typedef struct hashnode
{
    uint64_t count;
    UT_hash_handle hh;
} hashnode;

static hashnode hashnodes[131072];
static uint32_t hashnodescount;

static hashnode* hash;

static inline FORCEINLINE const char* nextpattern(const char* data, int len, node** prevnode, int* prevlen)
{
    while ((*prevlen) < len)
    {
        tcolor c = getcolor(data[*prevlen]);
        if ((*prevnode)->next[c])
        {
            *prevnode = (*prevnode)->next[c];
            ++(*prevlen);
            if ((*prevnode)->present)
                return data + (*prevlen);
        }
        else
        {
            break;
        }
    }
    return NULL;
}

static uint64_t findmatches(const char* data, int len)
{
    hashnode* hcheck;
    HASH_FIND(hh, hash, data, len, hcheck);
    if (hcheck)
        return hcheck->count;

    uint64_t count = 0;
    const char* ndata;
    node* prevnode = rootnode;
    int prevlen = 0;
    while ((ndata = nextpattern(data, len, &prevnode, &prevlen)) != NULL)
    {
        if (ndata == data + len)
        {
            ++count;
            break;
        }
        else
        {
            count += findmatches(ndata, len - (ndata - data));
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

    rootnode = nodestore + 0;
    nodestorecount = 1;

    int idx = 0;
    while (file.data[idx] != '\n')
    {
        node* n = rootnode;
        while (file.data[idx] != ',' && file.data[idx] != '\n')
        {
            tcolor c = getcolor(file.data[idx]);
            if (!n->next[c])
                n->next[c] = nodestore + (nodestorecount++);
            n = n->next[c];
            ++idx;
        }
        n->present = true;

        idx += 2; // ', ' or '\n\n'
        if (file.data[idx-1] == '\n')
            break;
    }

    uint64_t sum1 = 0, sum2 = 0;

    while (idx < fileSize - 2)
    {
        int laidx = idx;
        while (laidx < fileSize && file.data[laidx] != '\n')
            ++laidx;
        
        DEBUGLOG("%.*s --> ", laidx - idx, file.data + idx);
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
