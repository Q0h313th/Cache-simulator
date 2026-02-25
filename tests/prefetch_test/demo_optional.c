#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <x86intrin.h>
#include <time.h>

// Measure one access with fences
static inline uint64_t measure_access(int *p) {
    uint64_t t1, t2;
    _mm_lfence();
    t1 = __rdtsc();
    int x = *p;
    _mm_lfence();
    t2 = __rdtsc();
    return t2 - t1 + x * 0; // use x so compiler doesn't optimize it away
}

int main() {
    size_t size = 128UL * 1024 * 1024; // 128 MB >> typical LLC size
    int *buf;
    if (posix_memalign((void**)&buf, 64, size)) {
        perror("alloc");
        return 1;
    }

    // Touch all pages once so they are faulted in
    for (size_t i = 0; i < size/sizeof(int); i += 4096/sizeof(int))
        buf[i] = (int)i;

    srand(time(NULL));

    // HOT access: touch element once, then measure again
    int *hot = &buf[0];
    *hot = 123; // bring into cache
    uint64_t hot_cycles = measure_access(hot);

    // COLD access: pick random element far away, flush whole buffer
    // walk it in strides to thrash LLC
    for (size_t i = 0; i < size/sizeof(int); i += 64/sizeof(int))
        _mm_clflush(&buf[i]);

    int *cold = &buf[(rand() % (size/sizeof(int)))];
    uint64_t cold_cycles = measure_access(cold);

    printf("Hot (cached, L1) : %lu cycles\n", hot_cycles);
    printf("Cold (uncached, DRAM): %lu cycles\n", cold_cycles);

    free(buf);
    return 0;
}
