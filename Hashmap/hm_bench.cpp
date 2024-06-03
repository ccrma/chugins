// benchmark core/hashmap.c library against cpp unordered_map
/*
xxHash has highest bandwidth of the 3 provided: sipHash, murmurHash, xxHash3
https://github.com/Cyan4973/xxHash

Testing https://github.com/tidwall/hashmap.c
against std::unordered_map

https://martin.ankerl.com/2022/08/27/hashmap-bench-01/#benchmark-results-table
- benchmarks of cpp hashmaps
- took a look at https://github.com/Tessil/robin-map/tree/master/
- and https://github.com/ktprime/emhash
- but the code is ridiculous bullshit. even if they are faster, not worth the
- added complexity and compile time from the templates
- supposedly those impls are ~3x faster than cpp unordered_map on insert/erase
and find

Bench results
set            5000000 ops in 0.483 secs, 97 ns/op, 10350295 op/sec, 26.84
bytes/op set CPP        5000000 ops in 0.674 secs, 135 ns/op, 7414921 op/sec get
CPP        5000000 ops in 0.177 secs, 35 ns/op, 28237420 op/sec get 5000000 ops
in 0.189 secs, 38 ns/op, 26517884 op/sec iter           100 ops in 3.797 secs,
37967760 ns/op, 26 op/sec iter cpp       100 ops in 9.458 secs, 94577870 ns/op,
11 op/sec delete         5000000 ops in 0.339 secs, 68 ns/op, 14756358 op/sec
delete CPP     5000000 ops in 0.926 secs, 185 ns/op, 5398466 op/sec
set (cap)      5000000 ops in 0.294 secs, 59 ns/op, 17012184 op/sec
get (cap)      5000000 ops in 0.181 secs, 36 ns/op, 27574797 op/sec
delete (cap)   5000000 ops in 0.241 secs, 48 ns/op, 20756793 op/sec

- hashmap.c is faster than cpp unordered_map on insert by 30%
- hashmap.c is faster than cpp unordered_map on delete by 270%
- hashmap.c is slower than cpp unordered_map on find by ~6%

TODO: add bench tests for
- tsl::robin_map
- jg::dense_hash_map
- gtl::parallel_flat_map
*/

#include "hashmap.h"

#include <unordered_map>

static uintptr_t total_allocs = 0;
static uintptr_t total_mem = 0;

static void *xmalloc(size_t size)
{
    void *mem = malloc(sizeof(uintptr_t) + size);
    assert(mem);
    *(uintptr_t *)mem = size;
    total_allocs++;
    total_mem += size;
    return (char *)mem + sizeof(uintptr_t);
}

static void xfree(void *ptr)
{
    if (ptr)
    {
        total_mem -= *(uintptr_t *)((char *)ptr - sizeof(uintptr_t));
        free((char *)ptr - sizeof(uintptr_t));
        total_allocs--;
    }
}

static void shuffle(void *array, size_t numels, size_t elsize)
{
    char tmp[elsize];
    char *arr = (char *)array;
    for (size_t i = 0; i < numels - 1; i++)
    {
        int j = i + rand() / (RAND_MAX / (numels - i) + 1);
        memcpy(tmp, arr + j * elsize, elsize);
        memcpy(arr + j * elsize, arr + i * elsize, elsize);
        memcpy(arr + i * elsize, tmp, elsize);
    }
}

static int compare_ints_udata(const void *a, const void *b, void *udata)
{
    return *(int *)a - *(int *)b;
}

static uint64_t hash_int(const void *item, uint64_t seed0, uint64_t seed1)
{
    // tested xxhash3 is best
    return hashmap_xxhash3(item, sizeof(int), seed0, seed1);
    // return hashmap_sip(item, sizeof(int), seed0, seed1);
    // return hashmap_murmur(item, sizeof(int), seed0, seed1);
}

#define bench(name, N, code)                                              \
    {                                                                     \
        {                                                                 \
            if (strlen(name) > 0)                                         \
            {                                                             \
                printf("%-14s ", name);                                   \
            }                                                             \
            size_t tmem = total_mem;                                      \
            size_t tallocs = total_allocs;                                \
            uint64_t bytes = 0;                                           \
            clock_t begin = clock();                                      \
            for (int i = 0; i < N; i++)                                   \
            {                                                             \
                (code);                                                   \
            }                                                             \
            clock_t end = clock();                                        \
            double elapsed_secs = (double)(end - begin) / CLOCKS_PER_SEC; \
            double bytes_sec = (double)bytes / elapsed_secs;              \
            printf("%d ops in %.3f secs, %.0f ns/op, %.0f op/sec", N,     \
                   elapsed_secs, elapsed_secs / (double)N * 1e9,          \
                   (double)N / elapsed_secs);                             \
            if (bytes > 0)                                                \
            {                                                             \
                printf(", %.1f GB/sec", bytes_sec / 1024 / 1024 / 1024);  \
            }                                                             \
            if (total_mem > tmem)                                         \
            {                                                             \
                size_t used_mem = total_mem - tmem;                       \
                printf(", %.2f bytes/op", (double)used_mem / N);          \
            }                                                             \
            if (total_allocs > tallocs)                                   \
            {                                                             \
                size_t used_allocs = total_allocs - tallocs;              \
                printf(", %.2f allocs/op", (double)used_allocs / N);      \
            }                                                             \
            printf("\n");                                                 \
        }                                                                 \
    }

static void benchmarks(void)
{
    // random seed
    int seed = getenv("SEED") ? atoi(getenv("SEED")) : time(NULL);
    // bench size
    int N = getenv("N") ? atoi(getenv("N")) : 5000000;
    printf("seed=%d, count=%d, item_size=%zu\n", seed, N, sizeof(int));
    srand(seed);

    int *vals = (int *)xmalloc(N * sizeof(int));
    for (int i = 0; i < N; i++)
    {
        vals[i] = i;
    }

    shuffle(vals, N, sizeof(int));

    struct hashmap *map;
    std::unordered_map<int, int> cpp_map;

    shuffle(vals, N, sizeof(int));
    map = hashmap_new(sizeof(int), 0, seed, seed, hash_int, compare_ints_udata,
                      NULL, NULL);
    bench("set", N, {
        const int *v = (int *)hashmap_set(map, &vals[i]);
        // assert(!v);
    });

    bench("set CPP", N, {
        cpp_map[vals[i]] = vals[i];
        // assert(vals[i]);
    });

    shuffle(vals, N, sizeof(int));
    bench("get CPP", N, {
        int v = cpp_map[vals[i]];
        // assert(v == vals[i]);
    });
    bench("get", N, {
        const int *v = (int *)hashmap_get(map, &vals[i]);
        // assert(v && *v == vals[i]);
    });

    bench("iter", 100, {
        size_t iter = 0;
        void *item;
        int tmp;
        while (hashmap_iter(map, &iter, &item))
        {
            tmp = *(int *)item;
            assert(tmp <= N);
        }
    });

    bench("iter cpp", 100, {
        int tmp;
        for (auto &kv : cpp_map)
        {
            tmp = kv.second;
            assert(tmp <= N);
        }
    });

    shuffle(vals, N, sizeof(int));
    bench("delete", N, {
        const int *v = (int *)hashmap_delete(map, &vals[i]);
        // assert(v && *v == vals[i]);
    });
    bench("delete CPP", N, {
        int v = cpp_map.erase(vals[i]);
        // assert(v == 1);
    });

    hashmap_free(map);

    map = hashmap_new(sizeof(int), N, seed, seed, hash_int, compare_ints_udata,
                      NULL, NULL);
    bench("set (cap)", N, {
        const int *v = (int *)hashmap_set(map, &vals[i]);
        assert(!v);
    });

    shuffle(vals, N, sizeof(int));
    bench("get (cap)", N, {
        const int *v = (int *)hashmap_get(map, &vals[i]);
        assert(v && *v == vals[i]);
    });

    shuffle(vals, N, sizeof(int));
    bench("delete (cap)", N, {
        const int *v = (int *)hashmap_delete(map, &vals[i]);
        assert(v && *v == vals[i]);
    });

    hashmap_free(map);

    xfree(vals);

    if (total_allocs != 0)
    {
        fprintf(stderr, "total_allocs: expected 0, got %lu\n", total_allocs);
        exit(1);
    }
}

int main(void)
{
    hashmap_set_allocator(xmalloc, xfree);
    benchmarks();
}
