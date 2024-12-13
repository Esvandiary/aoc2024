#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"
#include "../common/uthash/uthash.h"

#include <limits.h>

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#if defined(_MSC_VER)
static inline FORCEINLINE uint64_t clz(uint64_t value)
{
    DWORD leading_zero = 0;
    if (_BitScanReverse64(&leading_zero, value))
        return 64 - leading_zero;
    else
        return 64;
}
#else
#define clz(n) __builtin_clzll(n)
#endif

static const uint64_t powers[20] = {
    0, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
    100000000, 1000000000, 10000000000ULL, 100000000000ULL,
    1000000000000ULL, 10000000000000ULL, 100000000000000ULL,
    1000000000000000ULL, 10000000000000000ULL, 100000000000000000ULL,
    1000000000000000000ULL, 10000000000000000000ULL,
};
static const uint8_t maxdigits[64] = {
    1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5,
    5, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 
    10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 15, 15,
    15, 16, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19
};

static inline FORCEINLINE uint64_t fast_log10(uint64_t n)
{
    const uint32_t bits = sizeof(n) * CHAR_BIT - clz(n);
    uint64_t digits = maxdigits[bits];
    if (n < powers[digits-1])
        --digits;
    return digits;
}

typedef struct lookup
{
    uint64_t value;
    uint64_t result;
    UT_hash_handle hh;
} lookup;

static lookup cachestore[1048576];
static uint64_t cachestorecount;

static lookup* cache[128];

static uint64_t count_stones(uint64_t n, uint32_t steps)
{
    if (steps == 0)
    {
        // DEBUGLOG("~ %" PRIu64 "\n", n);
        return 1;
    }

    uint64_t result;

    lookup* cacheresult;
    HASH_FIND_PTR(cache[steps], &n, cacheresult);
    if (cacheresult)
        return cacheresult->result;

    uint32_t digits;

    if (n == 0)
    {
        // DEBUGLOG("[%u] %" PRIu64 " --> zero\n", steps, n);
        result = count_stones(1, steps - 1);
    }
    else if ((digits = fast_log10(n)) & 1)
    {
        // DEBUGLOG("[%u] %" PRIu64 " --> odd digits (%u)\n", steps, n, digits);
        result = count_stones(n * 2024, steps - 1);
    }
    else
    {
        // DEBUGLOG("[%u] %" PRIu64 " --> even digits (%u)\n", steps, n, digits);
        const uint64_t n1 = n / powers[digits / 2], n2 = n % powers[digits / 2];
        result = count_stones(n1, steps - 1) + count_stones(n2, steps - 1);
    }

    cachestore[cachestorecount] = (lookup){ n, result };
    HASH_ADD_PTR(cache[steps], value, cachestore + cachestorecount);
    ++cachestorecount;

    return result;
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    uint64_t sum1 = 0, sum2 = 0;

    int idx = 0;
    while (idx < fileSize - 1)
    {
        int64_t num0 = 0;
        while (idx < fileSize && isdigit(file.data[idx]))
            num0 = (num0 * 10) + (file.data[idx++] & 0xF);
        ++idx; // ' '

        DEBUGLOG("### stone: %" PRId64 "\n", num0);
        const uint64_t result2 = count_stones(num0, 75);
        const uint64_t result1 = count_stones(num0, 25);
        sum1 += result1;
        sum2 += result2;
        DEBUGLOG("### num0 = %" PRId64 ", result1 = %" PRIu64 ", result2 = %" PRIu64 "\n", num0, result1, result2);
    }

    print_uint64(sum1);
    print_uint64(sum2);

    return 0;
}
