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

class MemoryHierarchy {
    protected:
        uint32_t l1_size_kb;
        uint32_t l1_assoc;
        uint32_t l2_assoc;
        uint32_t l3_assoc;
        CacheL1 l1;
        CacheL2 l2;
        CacheL3 l3;

    public: 
        MemoryHierarchy(uint32_t l1_size, uint32_t l1_associativity, uint32_t l2_associativity, uint32_t l3_associativity): 
            l1(l1_size, l1_associativity), 
            l2(4 * l1_size, l2_associativity), 
            l3(16 * l1_size, l3_associativity) {}

        bool access(uint64_t address) {
            if (!l1.access(address)) {
                if(!l2.access(address)) {
                    if(!l3.access(address)) {
                        return false;
                    }
                    return true;
                }
                return true;
            }
            return true;
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