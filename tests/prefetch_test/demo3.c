#include <stdio.h>
#include <x86intrin.h>

#define STRIDE 0x1000

char arr[10 * 0x1000];
__uint64_t start, finish;
char x;

int main() {
    /*
    for (int i = 0; i < 10 * 0x1000; i+= ARR_SIZE) {
        x = arr[i];
    }
    
    for (int i = 0; i < 10 * 0x1000; i += ARR_SIZE) {
        _mm_clflush(&arr[i]);
    }
    */
    
    for (int i = 0; i < 10 * STRIDE; i += STRIDE) {
        start = __rdtsc();
        _mm_lfence();
        x = arr[i];
        _mm_lfence();
        finish = __rdtsc();
        _mm_lfence();
        printf("index: %d \t cycles: %lu\n", i, finish - start);
    }
}