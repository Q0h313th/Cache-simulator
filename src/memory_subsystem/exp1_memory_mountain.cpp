#include "memory_hierarchy.h"

void memory_mountain(int working_set_kb, int num_passes, int stride){
    MemoryHierarchy mem(4, 4, 8, 16);
    uint64_t base = 0x00000000;

    int num_floats = working_set_kb * 1024 / 4;

    // INITIALIZE MEMORY FIRST!
    for (int j = 0; j < num_floats; j++){
        uint64_t address = base + (j * stride) * 4;
        mem.write_mem(address, 1.0f);
    }
    
    // reset the stats. no access is now a cold access
    //mem.print_occupancy();
    mem.reset_stats();  
    
    // do the passes
    for (int i = 0; i < num_passes; i++){
        for (int j = 0; j < num_floats; j++){
            uint64_t address = base + (j * stride) * 4;
            mem.read_mem(address);
        }
        //mem.get_l1_misses();
    }

    std::cout << "working set =" << std::setw(6) << working_set_kb << " KB | ";
    mem.print_stats_oneline();
    //mem.print_hits();
}

int main() {
    int working_set_size[] = {1, 2, 4, 8, 16, 32, 64, 128};
    int num_passes = 10;
    int stride = 1;
    for (int working_set: working_set_size) {
        memory_mountain(working_set, num_passes, stride);
    }
    return 0;
}