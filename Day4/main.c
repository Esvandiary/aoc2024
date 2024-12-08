#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#define INDEXOF(x,y) ((y)*linewidth + (x))
#define CHARAT(x,y) file.data[INDEXOF((x),(y))]

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int width;
    for (width = 0; file.data[width] != '\n'; ++width);
    const int linewidth = width + 1;
    const int height = fileSize / linewidth;

    uint64_t sum1 = 0, sum2 = 0;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (CHARAT(x,y) == 'X')
            {
                // is it chrismtas
                // cardinal
                if (x+3 < width && CHARAT(x+1,y) == 'M' && CHARAT(x+2,y) == 'A' && CHARAT(x+3,y) == 'S')
                    ++sum1;
                if (x-3 >= 0 && CHARAT(x-1,y) == 'M' && CHARAT(x-2,y) == 'A' && CHARAT(x-3,y) == 'S')
                    ++sum1;
                if (y+3 < height && CHARAT(x,y+1) == 'M' && CHARAT(x,y+2) == 'A' && CHARAT(x,y+3) == 'S')
                    ++sum1;
                if (y-3 >= 0 && CHARAT(x,y-1) == 'M' && CHARAT(x,y-2) == 'A' && CHARAT(x,y-3) == 'S')
                    ++sum1;
                // diagonal
                if (x+3 < width && y+3 < height && CHARAT(x+1,y+1) == 'M' && CHARAT(x+2,y+2) == 'A' && CHARAT(x+3,y+3) == 'S')
                    ++sum1;
                if (x-3 >= 0 && y+3 < height && CHARAT(x-1,y+1) == 'M' && CHARAT(x-2,y+2) == 'A' && CHARAT(x-3,y+3) == 'S')
                    ++sum1;
                if (x-3 >= 0 && y-3 >= 0 && CHARAT(x-1,y-1) == 'M' && CHARAT(x-2,y-2) == 'A' && CHARAT(x-3,y-3) == 'S')
                    ++sum1;
                if (x+3 < width && y-3 >= 0 && CHARAT(x+1,y-1) == 'M' && CHARAT(x+2,y-2) == 'A' && CHARAT(x+3,y-3) == 'S')
                    ++sum1;
            }
            else if (CHARAT(x,y) == 'A')
            {
                // is it mas
                if (x == 0 || x+1 >= width || y == 0 || y+1 >= height)
                    continue;
                if (((CHARAT(x-1,y-1) == 'M' && CHARAT(x+1,y+1) == 'S') || (CHARAT(x-1,y-1) == 'S' && CHARAT(x+1,y+1) == 'M'))
                    && ((CHARAT(x-1,y+1) == 'M' && CHARAT(x+1,y-1) == 'S') || (CHARAT(x-1,y+1) == 'S' && CHARAT(x+1,y-1) == 'M')))
                {
                    ++sum2;
                }
            }
        }
    }
    
    print_uint64(sum1);
    print_uint64(sum2);

    return 0;
}
