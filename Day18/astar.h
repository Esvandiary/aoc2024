#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../common/common.h"
#include "../common/uthash/uthash.h"

#define nodeindex(yc, xc, y, x) ((y)*(xc) + (x))

typedef struct astar_pos
{
    int32_t y;
    int32_t x;
} astar_pos;

typedef struct astar_node
{
    astar_pos pos;
    uint32_t blockcount;
    int64_t fScore;
    struct astar_node* prev;
    bool traversed;
    UT_hash_handle hh;
} astar_node;

#define MINHEAP_NAME astar
#define MINHEAP_TYPE astar_node
#define MINHEAP_SCORE(node) ((node)->fScore)
#include "../common/minheap.h"

static int16_t xdiff[4] = {0, -1, 0, 1};
static int16_t ydiff[4] = {-1, 0, 1, 0};

static astar_node* calculate(astar_node* nodes, int yc, int xc, astar_pos start, astar_pos goal)
{
    astar_minheap* openset = astar_minheap_init(8192);

    astar_node start_node = {
        .pos = (astar_pos) {.y = start.y, .x = start.x },
        .blockcount = 0,
        .fScore = 0,
        .prev = NULL,
        .traversed = false,
    };

    astar_minheap_insert(openset, &start_node);

    while (openset->heap_size)
    {
        astar_node* current = astar_minheap_extract_min(openset);
        // DEBUGLOG("got min node @ [%d,%d] with fScore %ld\n", current->pos.y, current->pos.x, current->fScore);
        if (current->pos.x == goal.x && current->pos.y == goal.y)
        {
            // DEBUGLOG("found score of %ld\n", current->fScore);
            return current;
        }

        if (current->traversed)
            continue;
        current->traversed = true;

        int32_t move_cost = 1;
        for (int i = 0; i < 4; ++i)
        {
            int cx = current->pos.x + xdiff[i];
            int cy = current->pos.y + ydiff[i];
            if (cx >= 0 && cy >= 0 && cx < xc && cy < yc)
            {
                astar_node* node = nodes + nodeindex(yc, xc, cy, cx);
                if (node->blockcount)
                    continue;
                int64_t newfscore = current->fScore + move_cost;
                if (newfscore > node->fScore)
                    continue;
                node->fScore = newfscore;
                node->prev = current;
                // DEBUGLOG("node @ [%d,%d] fScore now %ld\n", node->pos.y, node->pos.x, node->fScore);
                astar_minheap_insert(openset, node);
            }
        }
    }
    return NULL;
}
