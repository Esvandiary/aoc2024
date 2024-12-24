// #define ENABLE_DEBUGLOG
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <stdbool.h>

#if !defined(max)
#define max(a,b) ((a) < (b) ? (b) : (a))
#endif

typedef enum operation
{
    OP_OR = 1,
    OP_AND = 2,
    OP_XOR = 3,
} operation;

static inline FORCEINLINE uint32_t mkidxc(const char c1, const char c2, const char c3)
{
    uint32_t idx = 0;
    idx |= (uint32_t)(c1 & 0x1F) << 12;
    idx |= (c2 >= 64) ? ((uint32_t)(c2 & 0x1F) << 6) : ((0x20 | ((uint32_t)(c2 & 0xF))) << 6);
    idx |= (c3 >= 64) ? ((uint32_t)(c3 & 0x1F) << 0) : ((0x20 | ((uint32_t)(c3 & 0xF))) << 0);
    return idx;
}

static inline FORCEINLINE uint32_t mkidxs(const char* c)
{
    return mkidxc(*(c+0), *(c+1), *(c+2));
}


#if defined(ENABLE_DEBUGLOG)
static const char* opnames[] = { "(none)", "OR", "AND", "XOR" };

static char buf[4][16];
static uint8_t nextbuf;

static const char* idxtostr(uint32_t idx)
{
    char* b = buf[nextbuf++];
    b[0] = (idx >> 12) + 96;
    b[1] = ((idx >> 6) & 0x3F) < 32 ? (((idx >> 6) & 0x1F) + 96) : (((idx >> 6) & 0xF) + '0');
    b[2] = ((idx >> 0) & 0x3F) < 32 ? (((idx >> 0) & 0x1F) + 96) : (((idx >> 0) & 0xF) + '0');
    return b;
}
#endif


typedef struct instruction
{
    uint32_t in1;
    uint32_t in2;
    operation op;
    uint32_t out;
} instruction;

static uint8_t levels[27 << 12];

static instruction instructions[256][512];
static uint32_t instructionscount[256];

static uint8_t outputs[27 << 12];


int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int idx = 0;
    while (file.data[idx] != '\n')
    {
        uint32_t id = mkidxs(file.data + idx);
        idx += 5; // 'abc: '
        outputs[id] = file.data[idx] - '0';
        idx += 2; // '0\n'

        levels[id] = 1;
    }
    ++idx; // '\n'
    while (idx < fileSize - 2)
    {
        uint32_t in1 = mkidxs(file.data + idx);
        idx += 4;
        operation op;
        if (file.data[idx] == 'O')
        {
            op = OP_OR;
            idx += 3; // 'OR ';
        }
        else
        {
            op = (file.data[idx] == 'A' ? OP_AND : OP_XOR);
            idx += 4; // 'AND ';
        }
        uint32_t in2 = mkidxs(file.data + idx);
        idx += 7; // 'def -> ';
        uint32_t out = mkidxs(file.data + idx);
        idx += 4; // 'ghi\n';

        uint8_t lvl = 0;
        if (levels[in1] && levels[in2])
        {
            lvl = max(levels[in1], levels[in2]);
            levels[out] = lvl + 1;
        }

        DEBUGLOG("ix: %s %s %s -> %s\n", idxtostr(in1), opnames[op], idxtostr(in2), idxtostr(out));
        instructions[lvl][instructionscount[lvl]++] = (instruction) { in1, in2, op, out };
    }

    bool any = true;
    while (any)
    {
        any = false;
        for (int i = 0; i < instructionscount[0]; ++i)
        {
            const instruction ix = instructions[0][i];
            if (ix.op)
            {
                any = true;
                if (levels[ix.in1] && levels[ix.in2])
                {
                    uint8_t lvl = max(levels[ix.in1], levels[ix.in2]);
                    levels[ix.out] = lvl + 1;
                    instructions[lvl][instructionscount[lvl]++] = ix;
                    instructions[0][i].op = 0;
                }
            }
        }
    }

    for (int lvl = 0; instructionscount[lvl]; ++lvl)
    {
        for (int i = 0; i < instructionscount[lvl]; ++i)
        {
            const instruction ix = instructions[lvl][i];
            switch (ix.op)
            {
                case OP_OR:  outputs[ix.out] = outputs[ix.in1] | outputs[ix.in2]; break;
                case OP_AND: outputs[ix.out] = outputs[ix.in1] & outputs[ix.in2]; break;
                case OP_XOR: outputs[ix.out] = outputs[ix.in1] ^ outputs[ix.in2]; break;
            }
        }
    }

#if defined(ENABLE_DEBUGLOG)
    uint8_t mlvl = 0;
    for (int i = 0; i < 27 << 12; ++i)
        mlvl = max(mlvl, levels[i]);
    DEBUGLOG("max lvl: %u\n", mlvl);
#endif

    uint64_t sum1 = 0;
    for (char i = 0; i < 64; ++i)
    {
        sum1 |= ((uint64_t)outputs[mkidxc('z','0'+(i/10),'0'+(i%10))]) << i;
        DEBUGLOG("z%d%d: %u\n", i/10, i%10, outputs[mkidxc('z','0'+(i/10),'0'+(i%10))]);
    }

    print_uint64(sum1);

    return 0;
}
