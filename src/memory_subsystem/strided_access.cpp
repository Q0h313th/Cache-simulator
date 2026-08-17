#include "memory_hierarchy.h"

void strided_access(int stride, int num_accesses) {
    MemoryHierarchy mem(4, 4, 8, 16);
    uint64_t base = 0x00000000;

    for (int i = 0; i < num_accesses; i++) {
        uint64_t addresses = base + (i *  stride) * 4;
        mem.read_mem(addresses);
    }

    std::cout << "number of accesses per stride are: " << num_accesses << std::endl;
    std::cout << "stride =" << std::setw(4) << stride << " | ";
    mem.print_stats_oneline();
}


int main() {
    int num_accesses = 1024;
    for (int stride: {1, 2, 4, 8, 16, 32, 64}) {
        strided_access(stride, num_accesses);
    }
    return 0;

}