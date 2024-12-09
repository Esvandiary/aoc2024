#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define TYPE_ID   0
#define TYPE_FREE 1
#define TYPE_REF  2

typedef struct span
{
    int16_t id;
    uint8_t len;
    uint8_t type; // TYPE_*
} span;

static span pool[32768];
static uint64_t poolcount = 0;

static span spans[32768];
static uint64_t spancount = 0;

static span p1spans[32768];
static uint64_t p1spancount = 0;

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
    while (idx < fileSize && isdigit(file.data[idx]))
    {
        int file_len = file.data[idx++] - 48;
        int free_len = file.data[idx++] - 48;

        const span filespan = (span){file_id, file_len, TYPE_ID};
        const span freespan = (span){FREE, free_len, TYPE_FREE};
        spans[spancount++] = filespan;
        spans[spancount++] = freespan;
        p2tail = insert_node(p2tail, filespan);
        p2tail = insert_node(p2tail, freespan);

        ++file_id;
    }
    p2head = p2head->next;
    p2head->prev = NULL;

    int lastfile = spancount-1;
    while (spans[lastfile].type == TYPE_FREE)
        --lastfile;

    int firstfree = 0;
    while (spans[firstfree].type != TYPE_FREE)
    {
        p1spans[p1spancount++] = spans[firstfree];
        ++firstfree;
    }

    int p1lastfile = lastfile, p1firstfree = firstfree;

    span freespan = spans[p1firstfree];
    span lastspan = spans[p1lastfile];
    while (p1firstfree < p1lastfile)
    {
        DEBUGLOG("[P1] firstfree = %d, lastfile = %d\n", p1firstfree, p1lastfile);

        int count = min(lastspan.len, freespan.len);
        p1spans[p1spancount++] = (span){lastspan.id, count, TYPE_ID};
        lastspan.len -= count;
        freespan.len -= count;

        DEBUGLOG("[P1] moved %d of id %d (%d left), free len = %d, last len = %d\n", count, lastspan.id, lastspan.len, freespan.len, lastspan.len);

        if (freespan.len == 0)
        {
            ++p1firstfree;
            while (spans[p1firstfree].type != TYPE_FREE && p1firstfree < p1lastfile)
            {
                p1spans[p1spancount++] = spans[p1firstfree];
                ++p1firstfree;
            }
            freespan = spans[p1firstfree];
        }
        if (lastspan.len == 0)
        {
            --p1lastfile;
            while (spans[p1lastfile].type == TYPE_FREE)
                --p1lastfile;
            lastspan = spans[p1lastfile];
        }
    }
    if (lastspan.len != 0)
        p1spans[p1spancount++] = lastspan;

    uint64_t sum1 = 0;

    int p1len = 0;
    for (int i = 0; i < p1spancount; ++i)
    {
        for (int j = 0; j < p1spans[i].len; ++j)
            sum1 += (p1len++ * p1spans[i].id);
    }

#if defined(ENABLE_DEBUGLOG)
    DEBUGLOG("P1 ");
    for (int i = 0; i < p1spancount; ++i)
    {
        for (int j = 0; j < p1spans[i].len; ++j)
            DEBUGLOG("[%d]", p1spans[i].id);
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
