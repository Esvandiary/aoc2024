#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define CHARAT(x,y) (file.data[(y)*linewidth + (x)])
#define MAPCHARAT(x,y) (map.data[(y)*map.linewidth + (x)])

typedef struct map
{
    const char* data;
    int16_t linewidth;
    int16_t width;
    int16_t height;
} map;

typedef struct pos
{
    int16_t x;
    int16_t y;
} pos;

pos visited[1024];
int visitedcount = 0;

uint64_t trailheads(const map map, pos pos, char n)
{
    if (n > '9')
    {
        for (int i = 0; i < visitedcount; ++i)
            if (pos.x == visited[i].x && pos.y == visited[i].y)
                return ((uint64_t)1 << 32);
        visited[visitedcount++] = pos;
        return ((uint64_t)1 << 32) | 1;
    }

    uint64_t result = 0;
    if (pos.x > 0 && MAPCHARAT(pos.x-1, pos.y) == n)
        result += trailheads(map, (struct pos){pos.x-1, pos.y}, n+1);
    if (pos.x+1 < map.width && MAPCHARAT(pos.x+1, pos.y) == n)
        result += trailheads(map, (struct pos){pos.x+1, pos.y}, n+1);
    if (pos.y > 0 && MAPCHARAT(pos.x, pos.y-1) == n)
        result += trailheads(map, (struct pos){pos.x, pos.y-1}, n+1);
    if (pos.y+1 < map.height && MAPCHARAT(pos.x, pos.y+1) == n)
        result += trailheads(map, (struct pos){pos.x, pos.y+1}, n+1);
    return result;
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int width = 0;
    while (isdigit(file.data[width]))
        ++width;
    int linewidth = width;
    while (!isdigit(file.data[linewidth]))
        ++linewidth;
    int height = (fileSize+2) / linewidth;

    map map = (struct map){file.data, linewidth, width, height};

    uint64_t sum1 = 0, sum2 = 0;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (CHARAT(x, y) == '0')
            {
                visitedcount = 0;
                const uint64_t result = trailheads(map, (struct pos){x, y}, '1');
                const uint32_t score = (result & 0xFFFFFFFF);
                const uint32_t rating = (result >> 32);
                DEBUGLOG("trailhead @ [%d,%d] score %u rating %u\n", x, y, score, rating);
                sum1 += score;
                sum2 += rating;
            }
        }
    }

    print_uint64(sum1);
    print_uint64(sum2);

    return 0;
}
