#ifndef MEM_HIERARCHY_H
#define MEM_HIERARCHY_H

#include <iostream>
#include <cstdint>
#include <array>
#include <cstring>
#include "cache.h"
#include "MainMemory.h"

class MemoryHierarchy {
    protected:
        WritePolicy write_policy;
        CacheL1 l1;
        CacheL2 l2;
        CacheL3 l3;
        MainMemory memory;

    public:
        MemoryHierarchy(uint32_t l1_size, uint32_t l1_assoc, uint32_t l2_assoc, uint32_t l3_assoc,
                        WritePolicy wp = WRITE_BACK)
            : write_policy(wp),
              l1(l1_size,       l1_assoc, wp),
              l2(4  * l1_size,  l2_assoc, wp),
              l3(16 * l1_size,  l3_assoc, wp) {}

        float read_mem(uint64_t address) {
            cacheReadResult r = l1.read(address);
            if (r.hit) return r.value;

            r = l2.read(address);
            if (r.hit) {
                l1.cacheInstall(address, r.linedata, memory);
                return r.value;
            }

            r = l3.read(address);
            if (r.hit) {
                l1.cacheInstall(address, r.linedata, memory);
                l2.cacheInstall(address, r.linedata, memory);
                return r.value;
            }

            std::array<uint8_t, 64> block = memory.read_line(address);
            l1.cacheInstall(address, block, memory);
            l2.cacheInstall(address, block, memory);
            l3.cacheInstall(address, block, memory);
            float value;
            uint32_t offset = address & 63ULL;
            memcpy(&value, &block[offset], 4);
            return value;
        }

        void write_mem(uint64_t address, float value) {
            if (write_policy == WRITE_BACK) {
                bool hit = l1.write(address, value);
                if (hit) {
                    l1.set_dirty(address);
                    return;
                }
                // WRITE_ALLOCATE: patch value into block BEFORE installing
                std::array<uint8_t, 64> block = memory.read_line(address);
                uint32_t offset = address & 63ULL;
                memcpy(&block[offset], &value, sizeof(float)); // THE FIX
                l1.cacheInstall(address, block, memory);
                l2.cacheInstall(address, block, memory);
                l3.cacheInstall(address, block, memory);
                l1.set_dirty(address);
            } else {
                memory.write_float(address, value);
                l1.write(address, value);
            }
        }

        void print_stats() {
            std::cout << "=== L1 ===" << std::endl; l1.print_stats();
            std::cout << "=== L2 ===" << std::endl; l2.print_stats();
            std::cout << "=== L3 ===" << std::endl; l3.print_stats();
        }

        void print_stats_oneline() {
            std::cout << std::fixed << std::setprecision(2)
                    << "L1: " << l1.get_hit_rate() << "%  "
                    << "L2: " << l2.get_hit_rate() << "%  "
                    << "L3: " << l3.get_hit_rate() << "%" 
                    << std::endl;
        }

        void reset_stats() {
            l1.reset_stats();
            l2.reset_stats();
            l3.reset_stats();
        }

        void print_hits() {
            std::cout << "L1 | hits: " << l1.get_hits()
                    << " | misses: " << l1.get_misses()
                    << " | total: " << l1.get_total_accesses() << std::endl;
            std::cout << "L2 | hits: " << l2.get_hits()
                    << " | misses: " << l2.get_misses()
                    << " | total: " << l2.get_total_accesses() << std::endl;
            std::cout << "L3 | hits: " << l3.get_hits()
                    << " | misses: " << l3.get_misses()
                    << " | total: " << l3.get_total_accesses() << std::endl;
        }

        void get_l1_misses() {
            std::cout << "Misses: " << l1.get_misses() << std::endl;
        }

        void print_occupancy() {
            std::cout << "--- L1 occupancy ---" << std::endl;
            l1.print_occupancy();
            std::cout << "--- L2 occupancy ---" << std::endl;
            l2.print_occupancy();
            std::cout << "--- L3 occupancy ---" << std::endl;
            l3.print_occupancy();
        }
};

#endif