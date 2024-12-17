#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <stdbool.h>

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#if !defined(min)
#define min(x,y) (((x) < (y)) ? (x) : (y))
#endif

#define RA 0
#define RB 1
#define RC 2

typedef enum opcode
{
    OP_ADV = 0, // RA / 2**COP -> A
    OP_BXL = 1, // RB ^ OP -> B
    OP_BST = 2, // COP % 8 -> B
    OP_JNZ = 3, // (A != 0) ? jmp(OP) : nop
    OP_BXC = 4, // B ^ C -> B
    OP_OUT = 5, // print(COP % 8)
    OP_BDV = 6, // RA / 2**COP -> B
    OP_CDV = 7, // RA / 2**COP -> C
} opcode;

static const char* const opnames[] = {
    "adv",
    "bxl",
    "bst",
    "jnz",
    "bxc",
    "out",
    "bdv",
    "cdv"
};

static int64_t reg0[3];

static inline void printint(uint64_t n)
{
    int bits = n >> 58;
    n &= ((uint64_t)1 << 58) - 1;
    if (bits <= 0)
        return;

    bits -= 3;
    putchar(0x30 + ((n >> bits) & 7));
    bits -= 3;
    for (; bits >= 0; bits -= 3)
    {
        putchar(',');
        putchar(0x30 + ((n >> bits) & 7));
    }

    putchar('\n');
}

#define COMBOOP(operand) (((operand) < 4) ? (operand) : reg[(operand)-4])

#define ADDNUM(x, y) (((uint64_t)(x) << 3) | (uint64_t)(y))
#define SETBITS(x, n) ((uint64_t)(x) | ((uint64_t)(n) << 58))
#define GETNUM(x, n) (((uint64_t)(x) >> (((uint64_t)(x) >> 58) - (uint64_t)(n)*3 - 3)) & 0x7)

static uint64_t run(const char* data, const int dlen, int64_t reg[3])
{
#if defined(RUNSPAM)
    int64_t a = reg[RA], b = reg[RB], c = reg[RC];
#endif

    uint64_t outint = 0;
    uint8_t outintbits = 0;

    int idx = 0;
    while (idx + 3 <= dlen)
    {
        uint8_t opcode = data[idx] & 0xF;
        idx += 2;
        uint8_t operand = data[idx] & 0xF;
        idx += 2;

#if defined(RUNSPAM)
        if (RUNSPAM)
        {
            DEBUGLOG("[A:% 10" PRId64 "][B:% 10" PRId64 "][C:% 10" PRId64 "] processing: %s %u (%" PRId64 ")\n",
                reg[RA], reg[RB], reg[RC], opnames[opcode], operand, COMBOOP(operand));
        }
#endif

        switch (opcode)
        {
            case OP_ADV: // A / 2**COP -> A
                reg[RA] = reg[RA] / ((int64_t)1 << COMBOOP(operand));
                break;
            case OP_BXL: // B ^ OP -> B
                reg[RB] = reg[RB] ^ (int64_t)operand;
                break;
            case OP_BST: // COP % 8 -> B
                reg[RB] = COMBOOP(operand) & 7;
                break;
            case OP_JNZ: // (A != 0) ? jmp(OP) : nop
                if (reg[RA])
                    idx = operand*2;
                break;
            case OP_BXC: // B ^ C -> B
                reg[RB] = reg[RB] ^ reg[RC];
                break;
            case OP_OUT: // print(COP % 8)
                outint = ADDNUM(outint, COMBOOP(operand) & 7);
                outintbits += 3;
                break;
            case OP_BDV: // A / 2**COP -> B
                reg[RB] = reg[RA] / ((int64_t)1 << COMBOOP(operand));
                break;
            case OP_CDV: // A / 2**COP -> C
                reg[RC] = reg[RA] / ((int64_t)1 << COMBOOP(operand));
                break;
        }
    }

    return SETBITS(outint, outintbits);
}

static uint8_t p2ref[16] = {0};
static int p2refcount = 0;

static uint64_t p2refint;
static uint64_t p2refint2;

static uint64_t findp2(uint64_t p2, uint8_t n, const char* data, int dlen)
{
    int64_t reg[3];
    uint8_t ta = 0;
    uint64_t result = UINT64_MAX;
    for (ta = 0; ta < 8; ++ta)
    {
        uint64_t cpa = ADDNUM(p2, ta);
        reg[RA] = cpa; reg[RB] = reg0[RB]; reg[RC] = reg0[RC];
        DEBUGLOG("[P2] n %u, testing a = %" PRIu64 "\n", n, reg[RA]);
        uint64_t testp2 = run(data, dlen, reg);
        uint8_t testn = GETNUM(testp2, 0);
        uint8_t refn = GETNUM(p2refint, p2refcount - n - 1);
        DEBUGLOG("    testing %" PRIu64 " vs %u\n", (testp2 & 7), refn);
        if (testn == refn)
        {
            uint64_t curresult;
            if (n == p2refcount - 1)
                curresult = (testp2 == p2refint) ? cpa : UINT64_MAX;
            else
                curresult = findp2(cpa, n + 1, data, dlen);
            result = min(result, curresult);
        }
    }
    return result;
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int idx = 0;
    const int endidx = fileSize - ((file.data[fileSize-1] == '\n') ? 1 : 0);

    idx += 12; // 'Register A: '
    while (isdigit(file.data[idx]))
        reg0[RA] = (reg0[RA] * 10) + (file.data[idx++] & 0xF);
    idx += 13; // '\nRegister B: '
    while (isdigit(file.data[idx]))
        reg0[RB] = (reg0[RB] * 10) + (file.data[idx++] & 0xF);
    idx += 13; // '\nRegister C: '
    while (isdigit(file.data[idx]))
        reg0[RC] = (reg0[RC] * 10) + (file.data[idx++] & 0xF);
    idx += 11; // '\n\nProgram: '
    
    DEBUGLOG("start: A %" PRId64 ", B %" PRId64 ", C %" PRId64 "\n", reg0[RA], reg0[RB], reg0[RC]);

    // P1
    uint64_t p1 = run(file.data + idx, endidx - idx, reg0);
    printint(p1);

#if defined(ENABLE_DEBUGLOG) && defined(PROGSPAM)
    {
        static const char* combostr[] = {"0", "1", "2", "3", "A", "B", "C"};

        int cidx = idx;
        while (cidx + 2 < endidx)
        {
            uint8_t opcode = file.data[cidx] & 0xF;
            cidx += 2;
            uint8_t operand = file.data[cidx] & 0xF;
            cidx += 2;

            DEBUGLOG("%s %u | ", opnames[opcode], operand);
            switch (opcode)
            {
                case OP_ADV: DEBUGLOG("A = A / 2**%s\n", combostr[operand]); break;
                case OP_BXL: DEBUGLOG("B = B ^ %u\n", operand); break;
                case OP_BST: DEBUGLOG("B = %s %% 8\n", combostr[operand]); break;
                case OP_JNZ: DEBUGLOG("jnz(%u)\n", operand); break;
                case OP_BXC: DEBUGLOG("B = B ^ C\n"); break;
                case OP_OUT: DEBUGLOG("out(%s %% 8)\n", combostr[operand]); break;
                case OP_BDV: DEBUGLOG("B = A / 2**%s\n", combostr[operand]); break;
                case OP_CDV: DEBUGLOG("C = A / 2**%s\n", combostr[operand]); break;
            }
        }
    }
#endif

    // P2
    for (int cidx = idx; cidx < endidx; cidx += 2)
    {
        // add in reverse
        // p2refint |= (uint64_t)(file.data[cidx] & 7) << (p2refcount * 3);
        p2refint = ADDNUM(p2refint, file.data[cidx] & 7);
        p2ref[p2refcount++] = (uint64_t)(file.data[cidx] & 7);
    }
    p2refint = SETBITS(p2refint, p2refcount * 3);

#if defined(DEBUGLOG)
    DEBUGLOG("p2ref =");
    for (int i = 0; i < p2refcount; ++i)
        DEBUGLOG(" %u", p2ref[i]);
    DEBUGLOG("\n");
#endif

    uint64_t p2 = findp2(0, 0, file.data + idx, endidx - idx);

    int64_t reg[3] = {p2, reg0[1], reg0[2]};
    uint64_t v = run(file.data + idx, endidx - idx, reg);
    if ((v >> 58) % 2)
        p2 *= 8;
    print_uint64(p2);

    return 0;
}
