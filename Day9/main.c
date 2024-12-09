#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#if !defined(min)
    #define min(x,y) (((x) < (y)) ? (x) : (y))
#endif

#define TYPE_ID   0
#define TYPE_FREE 1
#define TYPE_REF  2

typedef struct span
{
    int16_t id;
    uint8_t len;
    uint8_t type; // TYPE_*
} span;

static int16_t nums[262144];

typedef struct llspan
{
    struct llspan* prev;
    struct llspan* next;
    span span;
} llspan;

static llspan p2nodes[32768];
static uint64_t p2nodecount = 0;

static inline FORCEINLINE llspan* create_llspan(span span)
{
    p2nodes[p2nodecount] = (llspan){NULL, NULL, span};
    return p2nodes + p2nodecount++;
}

static inline FORCEINLINE llspan* insert_node(llspan* after, span span)
{
    p2nodes[p2nodecount] = (llspan){after, after->next, span};
    if (after->next)
        after->next->prev = p2nodes + p2nodecount;
    after->next = p2nodes + p2nodecount;
    ++p2nodecount;
    return after->next;
}

#define FREE -1

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    llspan* p2head = create_llspan((span){FREE, 0, TYPE_FREE});
    llspan* p2tail = p2head;

    int idx = 0;
    int16_t file_id = 0;
    int numpos = 0;
    while (idx < fileSize && isdigit(file.data[idx]))
    {
        int file_len = file.data[idx++] - 48;
        int free_len = file.data[idx++] - 48;

        for (int i = 0; i < file_len; ++i)
            nums[numpos++] = file_id;
        for (int i = 0; i < free_len; ++i)
            nums[numpos++] = FREE;

        const span filespan = (span){file_id, file_len, TYPE_ID};
        const span freespan = (span){FREE, free_len, TYPE_FREE};
        p2tail = insert_node(p2tail, filespan);
        p2tail = insert_node(p2tail, freespan);

        ++file_id;
    }
    p2head = p2head->next;
    p2head->prev = NULL;

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
        if (nums[i] == FREE)
            break;
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


    llspan* p2firstfree = p2head;
    llspan* p2lastfile = p2tail;

    while (p2firstfree->span.type != TYPE_FREE)
        p2firstfree = p2firstfree->next;
    while (p2lastfile->span.type != TYPE_ID)
        p2lastfile = p2lastfile->prev;

    while (p2firstfree != p2lastfile)
    {
        DEBUGLOG("[P2] firstfree = %d, lastfile = %d\n", p2firstfree->span.id, p2lastfile->span.id);

        llspan* bigfree = p2firstfree;
        while (bigfree && bigfree != p2lastfile)
        {
            if (bigfree->span.type == TYPE_FREE && bigfree->span.len >= p2lastfile->span.len)
                break;
            bigfree = bigfree->next;
        }
        if (!bigfree || bigfree == p2lastfile)
        {
            // no matching free space, next file
            goto nextfile;
        }

        if (bigfree->span.len == p2lastfile->span.len)
        {
            // easymode: just replace
            span tmp = bigfree->span;
            bigfree->span = p2lastfile->span;
            p2lastfile->span = tmp;
        }
        else
        {
            uint8_t freecount = bigfree->span.len - p2lastfile->span.len;
            bigfree->span = p2lastfile->span;
            insert_node(bigfree, (span){FREE, freecount, TYPE_FREE});
            p2lastfile->span.type = TYPE_FREE;
        }

    nextfile:
        p2lastfile = p2lastfile->prev;
        while (p2lastfile && p2firstfree != p2lastfile && p2lastfile->span.type != TYPE_ID)
            p2lastfile = p2lastfile->prev;
        while (p2firstfree && p2firstfree != p2lastfile && p2firstfree->span.type != TYPE_FREE)
            p2firstfree = p2firstfree->next;
    }

#if defined(ENABLE_DEBUGLOG)
    DEBUGLOG("P2 ");
    for (llspan* node = p2head; node; node = node->next)
    {
        for (int j = 0; j < node->span.len; ++j)
        {
            if (node->span.type == TYPE_ID)
                DEBUGLOG("[%d]", node->span.id);
            else
                DEBUGLOG(".");
        }
    }
    DEBUGLOG("\n");
#endif

    uint64_t sum2 = 0;

    int p2len = 0;
    for (llspan* node = p2head; node; node = node->next)
    {
        if (node->span.type == TYPE_ID)
        {
            for (int j = 0; j < node->span.len; ++j)
                sum2 += (p2len++ * node->span.id);
        }
        else
        {
            p2len += node->span.len;
        }
    }

    print_uint64(sum2);

    return 0;
}
