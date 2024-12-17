// #define ENABLE_DEBUGLOG
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <stdbool.h>

#define isdigit(c) ((c) >= '0' && (c) <= '9')

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

static const char* opnames[] = {
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
static int64_t reg[3];

static inline FORCEINLINE int64_t comboop(uint8_t operand)
{
    return (operand < 4) ? operand : reg[operand-4];
}

static char outbuf1[8192];
static char outbuf1count;

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int idx = 0;

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

    memcpy(reg, reg0, sizeof(reg));

    const int idx0 = idx;
    const int lastidx = fileSize - ((file.data[fileSize-1] == '\n') ? 1 : 0);
    while (idx + 3 <= lastidx)
    {
        uint8_t opcode = file.data[idx] & 0xF;
        idx += 2;
        uint8_t operand = file.data[idx] & 0xF;
        idx += 2;

        DEBUGLOG("[A:% 10" PRId64 "][B:% 10" PRId64 "][C:% 10" PRId64 "] processing: %s %u (%" PRId64 ")\n",
            reg[RA], reg[RB], reg[RC], opnames[opcode], operand, comboop(operand));

        switch (opcode)
        {
            case OP_ADV: // A / 2**COP -> A
                reg[RA] = reg[RA] / ((int64_t)1 << comboop(operand));
                break;
            case OP_BXL: // B ^ OP -> B
                reg[RB] = reg[RB] ^ (int64_t)operand;
                break;
            case OP_BST: // COP % 8 -> B
                reg[RB] = comboop(operand) & 7;
                break;
            case OP_JNZ: // (A != 0) ? jmp(OP) : nop
                if (reg[RA])
                    idx = idx0 + operand*2;
                break;
            case OP_BXC: // B ^ C -> B
                reg[RB] = reg[RB] ^ reg[RC];
                break;
            case OP_OUT: // print(COP % 8)
                outbuf1[outbuf1count++] = ',';
                outbuf1[outbuf1count++] = 0x30 + (comboop(operand) & 7);
                break;
            case OP_BDV: // A / 2**COP -> B
                reg[RB] = reg[RA] / ((int64_t)1 << comboop(operand));
                break;
            case OP_CDV: // A / 2**COP -> C
                reg[RC] = reg[RA] / ((int64_t)1 << comboop(operand));
                break;
        }
    }

    printf("%.*s\n", outbuf1count - 1, outbuf1 + 1);

    return 0;
}
