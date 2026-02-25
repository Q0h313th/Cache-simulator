#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

int main() {
    int arr[10 * 0x1000];
    uint64_t start, finish;
    int x;

    //_mm_clflush((void const*)&arr[10 * 0x1000 / 4]);

    start = __rdtsc();
    _mm_lfence();
    x = arr[0x1000];  
    _mm_lfence();
    finish = __rdtsc();
    _mm_lfence();
    printf("unflushed uncached access: %lu cycles\n", finish - start);

    _mm_clflush((void const*)&arr[0x1000]);
    start = __rdtsc();
    _mm_lfence();
    int y = arr[0x1000];
    _mm_lfence();
    finish = __rdtsc();
    printf("flushed uncached access: %lu cycles\n", finish - start);
}