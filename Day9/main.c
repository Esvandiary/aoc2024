#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

static uint16_t freeid[32768][9];
static uint8_t freeidcount[32768];

#define FILEID(n) ((n) >> 1)

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    const int datasize = fileSize - ((file.data[fileSize-1] == '\n') ? 1 : 0);
    const int freecount = datasize / 2;
    const int filecount = datasize - freecount;

    const int lastfileidx = filecount * 2 - 2;

    DEBUGLOG("datasize = %d, filecount = %d, freecount = %d\n", datasize, filecount, freecount);

    uint64_t sum1 = 0, sum2 = 0;

    int idx = 1, ridx = lastfileidx;

    while (idx < ridx && isdigit(file.data[idx]))
    {
        int free_len = (file.data[idx] - 48) - freeidcount[idx];
        int laidx = idx;
        while (laidx < ridx && free_len)
        {
            int file_len = file.data[ridx] - 48;
            int lafreelen = (file.data[laidx] - 48) - freeidcount[laidx];
            DEBUGLOG("laidx = %d, ridx = %d, file len = %d, free len = %d\n", laidx, ridx, file_len, lafreelen);
            if (file_len <= lafreelen)
            {
                DEBUGLOG("[P2] move: %d len %d --> %d\n", FILEID(ridx), file_len, laidx);
                for (int i = 0; i < file_len; ++i)
                    freeid[laidx][freeidcount[laidx]++] = FILEID(ridx);
                freeidcount[ridx] = file_len;
                if (idx == laidx)
                    free_len -= file_len;
                ridx -= 2;
                laidx = idx;
            }
            else
            {
                laidx += 2;
                if (laidx >= ridx)
                {
                    // no match for this file, reset and continue
                    ridx -= 2;
                    laidx = idx;
                }
            }
        }
        idx += 2;
    }

    idx = 0; ridx = lastfileidx;
    int p1currlen = file.data[ridx] - 48;
    int p1numpos = 0, p2numpos = 0;

    while (idx <= lastfileidx && isdigit(file.data[idx]))
    {
        int file_len = file.data[idx] - 48;
        if (idx < ridx)
        {
            // P1: handle file
            for (int i = 0; i < file_len; ++i)
            {
                DEBUGLOG("[P1] [%d] FILE +++ %d * %d\n", p1numpos, p1numpos, FILEID(idx));
                sum1 += p1numpos++ * FILEID(idx);
            }
        }
        // P2: handle file
        if (!freeidcount[idx])
        {
            for (int i = 0; i < file_len; ++i)
            {
                DEBUGLOG("[P2] [%d] FILE +++ %d * %d\n", p2numpos, p2numpos, FILEID(idx));
                sum2 += p2numpos++ * FILEID(idx);
            }
        }
        else
        {
            for (int i = 0; i < freeidcount[idx]; ++i)
                DEBUGLOG("[P2] [%d] FREE\n", p2numpos + i);
            p2numpos += freeidcount[idx];
        }

        ++idx;

        int free_len = file.data[idx] - 48;

        // P2: apply any moved files
        int p2freelen = free_len;
        for (int i = 0; i < freeidcount[idx]; ++i)
        {
            DEBUGLOG("[P2] [%d] MOVD +++ %d * %u\n", p2numpos, p2numpos, freeid[idx][i]);
            sum2 += p2numpos++ * freeid[idx][i];
            --p2freelen;
        }
        for (int i = 0; i < p2freelen; ++i)
            DEBUGLOG("[P2] [%d] FREE\n", p2numpos + i);
        p2numpos += p2freelen;

        if (idx < ridx)
        {
            // P1: handle free
            while (idx < ridx && free_len)
            {
                while (p1currlen && free_len)
                {
                    DEBUGLOG("[P1] [%d] MOVD +++ %d * %d\n", p1numpos, p1numpos, FILEID(ridx));
                    sum1 += p1numpos++ * FILEID(ridx);
                    --p1currlen;
                    --free_len;
                }
                if (free_len)
                {
                    while (p1currlen == 0 && idx < ridx)
                    {
                        ridx -= 2;
                        p1currlen = file.data[ridx] - 48;
                    }
                }
            }
        }

        ++idx;
    }
    while (p1currlen)
    {
        DEBUGLOG("[P1] END +++ %d * %d\n", p1numpos, FILEID(ridx));
        sum1 += p1numpos++ * FILEID(ridx);
        --p1currlen;
    }

    print_uint64(sum1);
    print_uint64(sum2);

    return 0;
}
