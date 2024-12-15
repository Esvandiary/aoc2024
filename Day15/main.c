#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#include <stdbool.h>

#define POSX(idx) ((idx) % linewidth)
#define POSY(idx) ((idx) / linewidth)
#define IDXOF(x,y) ((y) * linewidth + (x))

#define P2POSX(idx) ((idx) % p2linewidth)
#define P2POSY(idx) ((idx) / p2linewidth)
#define P2IDXOF(x,y) ((y) * p2linewidth + (x))

char p2data[65536];
int p2size;
int p2linewidth;

bool p2canpush(int rxcur, int rycur, int dx, int dy)
{
    const char cur = p2data[P2IDXOF(rxcur, rycur)];
    if (cur == '.')
        return true;
    if (cur == '#')
        return false;
    if (dy == 0 || cur == '@')
    {
        // either horizontal push or i am robot
        return p2canpush(rxcur + dx, rycur + dy, dx, dy);
    }
    else
    {
        // vertical push of a box
        const int xleft = (cur == '[') ? rxcur : rxcur-1;
        const int xright = xleft + 1;
        return p2canpush(xleft, rycur + dy, dx, dy) && p2canpush(xright, rycur + dy, dx, dy);
    }
}

void p2dopush(int rxcur, int rycur, int dx, int dy, char prev)
{
    const char cur = p2data[P2IDXOF(rxcur, rycur)];
    if (cur == '.')
    {
        p2data[P2IDXOF(rxcur, rycur)] = prev;
        return;
    }
    if (dy == 0 || cur == '@')
    {
        // either horizontal push or i am robot
        p2dopush(rxcur + dx, rycur + dy, dx, dy, cur);
        p2data[P2IDXOF(rxcur, rycur)] = prev;
    }
    else if (cur == ']')
    {
        // vertical push of the right side of a box; delegate to the left side
        p2dopush(rxcur - 1, rycur, dx, dy, '.');
        p2data[P2IDXOF(rxcur, rycur)] = prev;
        return;
    }
    else
    {
        // vertical push of the left side of a box
        p2dopush(rxcur, rycur + dy, dx, dy, cur);
        p2dopush(rxcur + 1, rycur + dy, dx, dy, ']');
        p2data[P2IDXOF(rxcur, rycur)] = prev;
        p2data[P2IDXOF(rxcur + 1, rycur)] = '.';
    }
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int width = 0;
    while (file.data[width] == '#')
        ++width;
    int linewidth = width;
    while (file.data[linewidth] != '#')
        ++linewidth;

    int robotpos = linewidth;
    while (file.data[robotpos] != '@')
        ++robotpos;

    int rx0 = POSX(robotpos), ry0 = POSY(robotpos);

    int idx = robotpos;
    while (file.data[idx-1] != '\n' || file.data[idx] != '\n')
        ++idx;
    int height = idx / linewidth;

    int rx = rx0, ry = ry0;

    DEBUGLOG("width = %d, linewidth = %d, height = %d\n", width, linewidth, height);
    DEBUGLOG("robotpos = %d --> [%d,%d]\n", robotpos, rx, ry);

    // copy out data for P2 before we start yeeting boxes
    for (int i = 0; i < idx; ++i)
    {
        const char c = file.data[i];
        switch (c)
        {
            case '\n':
                p2data[p2size++] = '\n';
                break;
            case '@':
                p2data[p2size++] = '@';
                p2data[p2size++] = '.';
                break;
            case 'O':
                p2data[p2size++] = '[';
                p2data[p2size++] = ']';
                break;
            default:
                p2data[p2size++] = c;
                p2data[p2size++] = c;
                break;
        }
    }

    ++idx;

    // P1
    int startidx = idx;
    while (idx < fileSize)
    {
        int rdx, rdy;
        switch (file.data[idx])
        {
        case '^': rdx = 0; rdy = -1; break;
        case '<': rdx = -1; rdy = 0; break;
        case 'v': rdx = 0; rdy = 1; break;
        case '>': rdx = 1; rdy = 0; break;
        default: goto nextiter;
        }

        // check what's in that direction
        int rxnext = rx + rdx, rynext = ry + rdy;
        char nc = file.data[IDXOF(rxnext, rynext)];
        if (nc == '.')
            goto wemove;
        if (nc == '#')
            goto nextiter;

        // it's just a box
        int rxbox = rxnext, rybox = rynext;
        char cbox;
        do
        {
            rxbox += rdx;
            rybox += rdy;
        } while ((cbox = file.data[IDXOF(rxbox, rybox)]) == 'O');
        
        if (cbox == '#')
            goto nextiter;
        
        file.data[IDXOF(rxbox, rybox)] = 'O';
    wemove:
        file.data[IDXOF(rxnext, rynext)] = '@';
        file.data[IDXOF(rx, ry)] = '.';
        rx = rxnext;
        ry = rynext;
    nextiter:
        ++idx;

#if defined(ENABLE_DEBUGLOG) && defined(SPAM)
        DEBUGLOG("[%d]\n%.*s\n", idx - startidx, startidx - 2, file.data);
#endif
    }

    uint64_t sum1 = 0;

    for (int i = linewidth; i < linewidth*(height-1); ++i)
    {
        if (file.data[i] == 'O')
            sum1 += POSY(i)*100 + POSX(i);
    }

    print_uint64(sum1);

    // P2
    int p2width = width * 2;
    p2linewidth = p2width + (linewidth - width);

    idx = startidx;
    rx = rx0 * 2; ry = ry0;
    while (idx < fileSize)
    {
        int rdx, rdy;
        switch (file.data[idx])
        {
        case '^': rdx = 0; rdy = -1; break;
        case '<': rdx = -1; rdy = 0; break;
        case 'v': rdx = 0; rdy = 1; break;
        case '>': rdx = 1; rdy = 0; break;
        default: goto p2nextiter;
        }

        // check what's in that direction
        int rxnext = rx + rdx, rynext = ry + rdy;
        char nc = p2data[P2IDXOF(rxnext, rynext)];
        if (nc == '.')
        {
            p2data[P2IDXOF(rxnext, rynext)] = '@';
            p2data[P2IDXOF(rx, ry)] = '.';
            goto p2wemove;
        }
        if (nc == '#')
            goto p2nextiter;

        // it's a STUPID TWO-WIDE box
        if (p2canpush(rx, ry, rdx, rdy))
            p2dopush(rx, ry, rdx, rdy, '.');
        else
            goto p2nextiter;

    p2wemove:
        rx = rxnext;
        ry = rynext;
    p2nextiter:
        ++idx;

#if defined(ENABLE_DEBUGLOG) && defined(SPAM)
        DEBUGLOG("[%d]\n%.*s\n", idx - startidx, p2size - 1, p2data);
#endif
    }

    uint64_t sum2 = 0;

    for (int i = p2linewidth; i < p2linewidth*(height-1); ++i)
    {
        if (p2data[i] == '[')
            sum2 += P2POSY(i)*100 + P2POSX(i);
    }

    print_uint64(sum2);

    return 0;
}
