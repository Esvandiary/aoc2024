// #define ENABLE_DEBUGLOG
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <stdbool.h>

#if !defined(max)
#define max(a,b) ((a) < (b) ? (b) : (a))
#endif

#if defined(_MSC_VER)
static inline FORCEINLINE uint64_t ctz(uint64_t value)
{
    DWORD leading_zero = 0;
    if (_BitScanForward64(&leading_zero, value))
        return leading_zero;
    else
        return 64;
}
#else
#define ctz(n) __builtin_ctzll(n)
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

static inline uint32_t inczidx(uint32_t idx)
{
    uint32_t i2 = (idx >> 6) & 0xF, i3 = (idx >> 0) & 0xF;
    if (i3 == 9)
    {
        i3 = 0;
        ++i2;
    }
    else
    {
        ++i3;
    }
    return (idx & 0x3F000) | ((0x20 | i2) << 6) | (0x20 | i3);
}

static inline uint32_t deczidx(uint32_t idx)
{
    uint32_t i2 = (idx >> 6) & 0xF, i3 = (idx >> 0) & 0xF;
    if (i3 == 0)
    {
        i3 = 9;
        --i2;
    }
    else
    {
        --i3;
    }
    return (idx & 0x3F000) | ((0x20 | i2) << 6) | (0x20 | i3);
}

static const char* opnames[] = { "(none)", "OR", "AND", "XOR" };

static char buf[16][4];
static uint8_t nextbuf;

static const char* idxtostr(uint32_t idx)
{
    char* b = buf[nextbuf];
    nextbuf = (nextbuf + 1) % 16;
    b[0] = (idx >> 12) + 96;
    b[1] = ((idx >> 6) & 0x3F) < 32 ? (((idx >> 6) & 0x1F) + 96) : (((idx >> 6) & 0xF) + '0');
    b[2] = ((idx >> 0) & 0x3F) < 32 ? (((idx >> 0) & 0x1F) + 96) : (((idx >> 0) & 0xF) + '0');
    return b;
}


typedef struct instruction
{
    uint32_t in1;
    uint32_t in2;
    operation op;
    uint32_t out;
} instruction;

static uint8_t p1levels[27 << 12];
static uint8_t p2levels[27 << 12];

static instruction p1instructions[256][256];
static uint32_t p1instructionscount[256];

static instruction p2instructions[256][256];
static uint32_t p2instructionscount[256];

static uint8_t specoutputs[27 << 12];
static uint8_t p1outputs[27 << 12];
static uint8_t p2outputs[27 << 12];

static uint32_t p2wrongmids[16];
static uint32_t p2wrongmidscount;
static uint32_t p2wrongends[16];
static uint32_t p2wrongendscount;

static uint32_t p2ends[64];

static void run(instruction (* instructions)[256], uint32_t* instructionscount, uint8_t* outputs)
{
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
}

static uint32_t find_zoutput(const instruction* instructions, uint32_t instructionscount, uint32_t num, uint32_t depth)
{
    const instruction ix = instructions[num];
    DEBUGLOG("find_zoutput for %s\n", idxtostr(ix.out));
    if ((ix.out >> 12) == 26)
    {
        const uint32_t decz = deczidx(ix.out);
        DEBUGLOG("got z: %s --> %s\n", idxtostr(ix.out), idxtostr(decz));
        for (int i = 0; i < p2wrongendscount; ++i)
            if (decz == p2instructions[0][p2wrongends[i]].out)
                return decz;
        return 0;
    }

    if (depth == 0)
        return 0;
    
    for (int j = 0; j < instructionscount; ++j)
    {
        if (instructions[j].in1 == ix.out || instructions[j].in2 == ix.out)
        {
            uint32_t result = find_zoutput(instructions, instructionscount, j, depth - 1);
            if (result)
                return result;
        }
    }
    return 0;
}

static void fixup_levels(instruction (*instructions)[256], uint32_t* instructionscount, uint8_t* levels)
{
    int processed = 1;
    while (processed)
    {
        processed = 0;
        for (int i = 0; i < instructionscount[0]; ++i)
        {
            const instruction ix = instructions[0][i];
            if (ix.op)
            {
                ++processed;
                // DEBUGLOG("remaining: %s %s %s --> %s\n", idxtostr(ix.in1), opnames[ix.op], idxtostr(ix.in2), idxtostr(ix.out));
                if (levels[ix.in1] && levels[ix.in2])
                {
                    // ++processed;
                    uint8_t lvl = max(levels[ix.in1], levels[ix.in2]);
                    // DEBUGLOG("updating to lvl %u: %s %s %s --> %s\n", lvl+1, idxtostr(ix.in1), opnames[ix.op], idxtostr(ix.in2), idxtostr(ix.out));
                    levels[ix.out] = lvl + 1;
                    instructions[lvl][instructionscount[lvl]++] = ix;
                    instructions[0][i].op = 0;
                }
            }
        }
    }
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int idx = 0;
    while (file.data[idx] != '\n')
    {
        uint32_t id = mkidxs(file.data + idx);
        idx += 5; // 'abc: '
        specoutputs[id] = p1outputs[id] = p2outputs[id] = file.data[idx] - '0';
        idx += 2; // '0\n'

        p1levels[id] = p2levels[id] = 1;
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

        DEBUGLOG("ix: %s %s %s -> %s\n", idxtostr(in1), opnames[op], idxtostr(in2), idxtostr(out));
        p1instructions[0][p1instructionscount[0]++] = (instruction) { in1, in2, op, out };
        p2instructions[0][p2instructionscount[0]++] = (instruction) { in1, in2, op, out };

        const bool isstart = ((in1 >> 12) == 24) || ((in1 >> 12) == 25);
        const bool isend = ((out >> 12) == 26);
        if (isend)
            p2ends[((out >> 6) & 0xF) * 10 + (out & 0xF)] = p2instructionscount[0] - 1;
        if (isend && op != OP_XOR && out != mkidxc('z', '4', '5'))
            p2wrongends[p2wrongendscount++] = p2instructionscount[0] - 1;
        if (!isstart && !isend && op == OP_XOR)
            p2wrongmids[p2wrongmidscount++] = p2instructionscount[0] - 1;
    }

    fixup_levels(p1instructions, p1instructionscount, p1levels);

    run(p1instructions, p1instructionscount, p1outputs);

#if defined(ENABLE_DEBUGLOG)
    uint8_t mlvl = 0;
    for (int i = 0; i < 27 << 12; ++i)
        mlvl = max(mlvl, p1levels[i]);
    DEBUGLOG("max lvl: %u\n", mlvl);
#endif

    uint64_t sum1 = 0;
    for (char i = 0; i < 64; ++i)
        sum1 |= ((uint64_t)p1outputs[mkidxc('z','0'+(i/10),'0'+(i%10))]) << i;

    print_uint64(sum1);

    uint64_t x = 0;
    uint64_t y = 0;
    for (char i = 0; i < 64; ++i)
    {
        x |= ((uint64_t)specoutputs[mkidxc('x','0'+(i/10),'0'+(i%10))]) << i;
        y |= ((uint64_t)specoutputs[mkidxc('y','0'+(i/10),'0'+(i%10))]) << i;
    }

    DEBUGLOG("x = %" PRIu64 ", y = %" PRIu64 ", x+y = %" PRIu64 ", z = %" PRIu64 "\n", x, y, x+y, sum1);

    DEBUGLOG("wrong mids: %u, wrong ends: %u\n", p2wrongmidscount, p2wrongendscount);

    for (int i = 0; i < p2wrongendscount; ++i)
        DEBUGLOG("wrong end: %s\n", idxtostr(p2instructions[0][p2wrongends[i]].out));

    uint32_t wrong[8];
    uint32_t wrongcount = 0;
    for (int i = 0; i < p2wrongmidscount; ++i)
    {
        instruction mix = p2instructions[p2wrongmids[i] >> 24][p2wrongmids[i] & 16777215];
        uint32_t prevz = 0, depth = 2;
        while (!prevz)
            prevz = find_zoutput(p2instructions[0], p2instructionscount[0], p2wrongmids[i], depth++);
        DEBUGLOG("p2 wrong mid %d prev z = %s (%u)\n", i, idxtostr(prevz), prevz);
        uint32_t prevzidx = p2ends[((prevz >> 6) & 0xF) * 10 + (prevz & 0xF)];
        instruction eix = p2instructions[prevzidx >> 24][prevzidx & 16777215];
        DEBUGLOG("p2 wrong mid %d (%s) matches %s\n", i, idxtostr(mix.out), idxtostr(eix.out));
        DEBUGLOG("mix: %s %s %s -> %s\n", idxtostr(mix.in1), opnames[mix.op], idxtostr(mix.in2), idxtostr(mix.out));
        DEBUGLOG("eix: %s %s %s -> %s\n", idxtostr(eix.in1), opnames[eix.op], idxtostr(eix.in2), idxtostr(eix.out));

        DEBUGLOG("instructions at lvl %u, %u\n", p2wrongmids[i] >> 24, prevzidx >> 24);

        wrong[wrongcount++] = mix.out;
        wrong[wrongcount++] = eix.out;

        p2instructions[p2wrongmids[i] >> 24][p2wrongmids[i] & 16777215].out = eix.out;
        p2instructions[prevzidx >> 24][prevzidx & 16777215].out = mix.out;
    }

    fixup_levels(p2instructions, p2instructionscount, p2levels);

    run(p2instructions, p2instructionscount, p2outputs);

    uint64_t z2 = 0;
    for (char i = 0; i < 64; ++i)
        z2 |= ((uint64_t)p2outputs[mkidxc('z','0'+(i/10),'0'+(i%10))]) << i;

    uint64_t diff = (z2 > x+y) ? (z2 - (x+y)) : ((x+y) - z2);
    uint64_t diffbit = ctz(diff);
    DEBUGLOG("part-fixed result: %" PRIu64 " (diff %" PRIu64 ", diff bit %" PRIu64 ")\n", z2, diff, diffbit);

    uint32_t xbad = mkidxc('x', diffbit/10 + '0', diffbit%10 + '0');
    uint32_t ybad = mkidxc('y', diffbit/10 + '0', diffbit%10 + '0');

#if defined(ENABLE_DEBUGLOG)
    instruction* badand = NULL;
    instruction* badxor = NULL;
#endif

    for (int i = 0; i < p2instructionscount[1]; ++i)
    {
        const instruction ix = p2instructions[1][i];
        if ((ix.in1 == xbad || ix.in1 == ybad) && (ix.in2 == xbad || ix.in2 == ybad))
        {
            if (ix.op == OP_AND)
                wrong[wrongcount++] = ix.out;
            else if (ix.op == OP_XOR)
                wrong[wrongcount++] = ix.out;

#if defined(ENABLE_DEBUGLOG)
            if (ix.op == OP_AND)
                badand = p2instructions[1] + i;
            else if (ix.op == OP_XOR)
                badxor = p2instructions[1] + i;
#endif

            if (wrongcount == 8)
                break;
        }
    }

#if defined(ENABLE_DEBUGLOG)
    uint32_t tmp = badand->out;
    badand->out = badxor->out;
    badxor->out = tmp;

    memcpy(p2outputs, specoutputs, sizeof(p2outputs));
    run(p2instructions, p2instructionscount, p2outputs);

    uint64_t z3 = 0;
    for (char i = 0; i < 64; ++i)
        z3 |= ((uint64_t)p2outputs[mkidxc('z','0'+(i/10),'0'+(i%10))]) << i;

    DEBUGLOG("z3 = %" PRIu64 "\n", z3);
#endif

    uint32_t last = 0;
    uint32_t best;
    uint32_t swrong[8];
    uint32_t swrongcount = 0;
    for (int o = 0; o < 8; ++o)
    {
        best = UINT32_MAX;
        for (int i = 0; i < 8; ++i)
        {
            if (wrong[i] < best && wrong[i] > last)
                best = wrong[i];
        }
        last = best;
        swrong[swrongcount++] = best;
    }

    printf("%s", idxtostr(swrong[0]));
    for (int i = 1; i < 8; ++i)
        printf(",%s", idxtostr(swrong[i]));
    printf("\n");

    return 0;
}
