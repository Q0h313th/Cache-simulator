#include <stdio.h>
#include <x86intrin.h>

char arr[64]; // 40KiB array
__uint64_t start, finish;
char x;

int main() {
    for (int i = 0; i < 64; i++) {
        _mm_clflush(&arr[0]);
        start = __rdtsc();
        _mm_lfence();
        x = arr[i];
        _mm_lfence();
        finish = __rdtsc();
        _mm_lfence();
        printf("index: %d \t cycles: %lu\n", i, finish - start);
    }
}