#ifndef MEM_HIERARCHY_H
#define MEM_HIERARCHY_H

#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <sstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include "cache.h"
#include "MainMemory.h"
#include <array>

class MemoryHierarchy {
    protected:
        uint32_t l1_size_kb;
        uint32_t l1_assoc;
        uint32_t l2_assoc;
        uint32_t l3_assoc;
        CacheL1 l1;
        CacheL2 l2;
        CacheL3 l3;
        MainMemory memory;

    public: 
        MemoryHierarchy(uint32_t l1_size, uint32_t l1_associativity, uint32_t l2_associativity, uint32_t l3_associativity): 
            l1(l1_size, l1_associativity), 
            l2(4 * l1_size, l2_associativity), 
            l3(16 * l1_size, l3_associativity) {}

        float read_mem(uint64_t address) {
            cacheReadResult result = l1.read(address);
            if (result.hit) {
                return result.value;
            }
            result = l2.read(address);
            if(result.hit) {
                // install cache line into L1
                l1.cacheInstall(address, result.linedata);
                return result.value;
            }
            result = l3.read(address);
            if (result.hit) {
                l1.cacheInstall(address, result.linedata);
                l2.cacheInstall(address, result.linedata);
                return result.value;
            }
            std::array<uint8_t, 64> block;
            block = memory.read_line(address);
            l1.cacheInstall(address, block);
            l2.cacheInstall(address, block);
            l3.cacheInstall(address, block);
            float value;
            uint32_t offset = address & 63ULL;
            memcpy(&value, &block[offset], 4);
            return value;
        }

        void print_stats() {
            std::cout << "=== L1 ===" << std::endl;
            l1.print_stats();
            std::cout << "=== L2 ===" << std::endl;
            l2.print_stats();
            std::cout << "=== L3 ===" << std::endl;
            l3.print_stats();
        }
};


#endif