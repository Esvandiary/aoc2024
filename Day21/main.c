// #define ENABLE_DEBUGLOG
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <stdbool.h>

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define MKIDX(x,y) { (x), (y) }
#define KPIDX(x,y) (((y) << 2) | (x))
#define KPPOSX(idx) ((idx) & 0x3)
#define KPPOSY(idx) (((idx) >> 2) & 0x3)

typedef struct pos
{
    uint8_t x;
    uint8_t y;
} pos;

#define KP_UP    KPIDX(1,0)
#define KP_LEFT  KPIDX(0,1)
#define KP_DOWN  KPIDX(1,1)
#define KP_RIGHT KPIDX(2,1)
#define KP_A     KPIDX(2,0)

static const pos mppos[] = {
    MKIDX(1,3),
    MKIDX(0,2), MKIDX(1,2), MKIDX(2,2),
    MKIDX(0,1), MKIDX(1,1), MKIDX(2,1),
    MKIDX(0,0), MKIDX(1,0), MKIDX(2,0),
    MKIDX(2,3)
};
#define MP_A 10

typedef struct state
{
    pos pos[26];
} state;

static inline FORCEINLINE bool allowxy(int lvl, pos p, pos np)
{
    if (lvl == 0)
        return (np.x != 0 || p.y != 3);
    else
        return (np.x != 0 || p.y != 0);
}

static inline FORCEINLINE bool allowyx(int lvl, pos p, pos np)
{
    if (lvl == 0)
        return (np.y != 3 || p.x != 0);
    else
        return (np.y != 0 || p.x != 0);
}

static uint64_t get_min_len(int lvl, int maxlvl, state* s, char digit)
{
    if (lvl == maxlvl)
        return 1;

    uint64_t result = 0;
    pos newpos = (lvl == 0) ? mppos[digit] : (pos) { KPPOSX(digit), KPPOSY(digit) };
    if (newpos.x == s->pos[lvl].x && newpos.y == s->pos[lvl].y)
    {
        DEBUGLOG("%.*s[%d] (%u,%u) --> (%u,%u), in position already\n", lvl*4, spaces, lvl, s->pos[lvl].x, s->pos[lvl].y, newpos.x, newpos.y);
        result += get_min_len(lvl + 1, maxlvl, s, KP_A);
    }
    else if (newpos.x == s->pos[lvl].x)
    {
        // Y only
        DEBUGLOG("%.*s[%d] (%u,%u) --> (%u,%u), Y only\n", lvl*4, spaces, lvl, s->pos[lvl].x, s->pos[lvl].y, newpos.x, newpos.y);
        int ydiff = (newpos.y > s->pos[lvl].y) ? 1 : -1;
        char ychar = (newpos.y > s->pos[lvl].y) ? KP_DOWN : KP_UP;
        uint64_t yscore = 0;
        for (; s->pos[lvl].y != newpos.y; s->pos[lvl].y += ydiff)
            yscore += get_min_len(lvl + 1, maxlvl, s, ychar);
        yscore += get_min_len(lvl + 1, maxlvl, s, KP_A);
        DEBUGLOG("%.*s  Y score: %u\n", lvl*4, spaces, yscore);
        result += yscore;
    }
    else if (newpos.y == s->pos[lvl].y)
    {
        // X only
        DEBUGLOG("%.*s[%d] (%u,%u) --> (%u,%u), X only\n", lvl*4, spaces, lvl, s->pos[lvl].x, s->pos[lvl].y, newpos.x, newpos.y);
        int xdiff = (newpos.x > s->pos[lvl].x) ? 1 : -1;
        char xchar = (newpos.x > s->pos[lvl].x) ? KP_RIGHT : KP_LEFT;
        uint64_t xscore = 0;
        for (; s->pos[lvl].x != newpos.x; s->pos[lvl].x += xdiff)
            xscore += get_min_len(lvl + 1, maxlvl, s, xchar);
        xscore += get_min_len(lvl + 1, maxlvl, s, KP_A);
        DEBUGLOG("%.*s  X score: %u\n", lvl*4, spaces, xscore);
        result += xscore;
    }
    else
    {
        // guh
        DEBUGLOG("%.*s[%d] (%u,%u) --> (%u,%u), guh\n", lvl*4, spaces, lvl, s->pos[lvl].x, s->pos[lvl].y, newpos.x, newpos.y);
        int xdiff = (newpos.x > s->pos[lvl].x) ? 1 : -1;
        char xchar = (newpos.x > s->pos[lvl].x) ? KP_RIGHT : KP_LEFT;
        int ydiff = (newpos.y > s->pos[lvl].y) ? 1 : -1;
        char ychar = (newpos.y > s->pos[lvl].y) ? KP_DOWN : KP_UP;

        uint64_t minscore = UINT32_MAX;
        state mins;
        if (allowxy(lvl, s->pos[lvl], newpos))
        {
            state xys = *s;
            uint64_t xyscore = 0;
            for (; xys.pos[lvl].x != newpos.x; xys.pos[lvl].x += xdiff)
                xyscore += get_min_len(lvl + 1, maxlvl, &xys, xchar);
            for (; xys.pos[lvl].y != newpos.y; xys.pos[lvl].y += ydiff)
                xyscore += get_min_len(lvl + 1, maxlvl, &xys, ychar);

            xyscore += get_min_len(lvl + 1, maxlvl, &xys, KP_A);

            DEBUGLOG("%.*s  XY score: %u\n", lvl*4, spaces, xyscore);
            if (xyscore < minscore)
            {
                minscore = xyscore;
                mins = xys;
            }
        }
        if (allowyx(lvl, s->pos[lvl], newpos))
        {
            state yxs = *s;
            uint64_t yxscore = 0;
            for (; yxs.pos[lvl].y != newpos.y; yxs.pos[lvl].y += ydiff)
                yxscore += get_min_len(lvl + 1, maxlvl, &yxs, ychar);
            for (; yxs.pos[lvl].x != newpos.x; yxs.pos[lvl].x += xdiff)
                yxscore += get_min_len(lvl + 1, maxlvl, &yxs, xchar);
            yxscore += get_min_len(lvl + 1, maxlvl, &yxs, KP_A);

            DEBUGLOG("%.*s  YX score: %u\n", lvl*4, spaces, yxscore);
            if (yxscore < minscore)
            {
                minscore = yxscore;
                mins = yxs;
            }
        }

        result += minscore;
        *s = mins;
    }

    return result;
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    uint64_t sum1 = 0, sum2 = 0;

    int idx = 0;
    while (idx + 2 < fileSize)
    {
        char digits[8];
        int digitcount = 0;
        uint64_t num1 = 0;
        while (isdigit(file.data[idx]))
        {
            DEBUGLOG("%c", file.data[idx]);
            digits[digitcount] = file.data[idx++] - '0';
            num1 = (num1 * 10) + digits[digitcount++];
        }
        digits[digitcount++] = MP_A;
        idx += 2; // 'A\n'

        DEBUGLOG("A: ");

        state s;
        s.pos[0] = (pos) { 2, 3 };
        s.pos[1] = (pos) { 2, 0 };
        s.pos[2] = (pos) { 2, 0 };

        uint64_t result = 0;
        for (int i = 0; i < digitcount; ++i)
            result += get_min_len(0, 3, &s, digits[i]);
        
        sum1 += (result * num1);
/*
        state s2;
        s2.pos[0] = (pos) { 2, 3 };
        for (int i = 1; i < 26; ++i)
            s2.pos[i] = (pos) { 2, 0 };

        uint64_t result2 = 0;
        for (int i = 0; i < digitcount; ++i)
            result2 += get_min_len(0, 26, &s2, digits[i]);
        
        sum2 += (result2 * num1);
*/
        DEBUGLOG("\n");
    }

    print_uint64(sum1);
    print_uint64(sum2);

    return 0;
}
