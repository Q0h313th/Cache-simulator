#include "memory_hierarchy.h"

int main() {
    const int N = 64;
    MemoryHierarchy mem(4, 4, 8, 16);

    uint64_t base_A = 0x00000000;
    uint64_t base_B = base_A + N * N * 4;
    uint64_t base_C = base_B + N * N * 4;

    // initialisation 
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            mem.write_mem(base_A + (i * N + j) * 4, 1.0f);
            mem.write_mem(base_B + (i * N + j) * 4, 1.0f);
            mem.write_mem(base_C + (i * N + j) * 4, 0.0f);
        }
    }

    // matmul
    for (int i = 0; i < N; i++) {
        for (int k = 0; k < N; k++) {
            for (int j = 0; j < N; j++) {
                float a = mem.read_mem(base_A + (i * N + k) * 4);
                float b = mem.read_mem(base_B + (k * N + j) * 4);
                float c = mem.read_mem(base_C + (i * N + j) * 4);
                mem.write_mem(base_C + (i * N + j) * 4, c + a * b);
            }
        }
    }

    std::cout << "Matrix C:" << std::endl;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            float val = mem.read_mem(base_C + (i*N + j)*4);
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    mem.print_stats();
    return 0;
}