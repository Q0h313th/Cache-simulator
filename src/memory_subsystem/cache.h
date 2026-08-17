#ifndef CACHE_H
#define CACHE_H

#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <sstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <cstring>
#include <array>
#include "MainMemory.h"

enum WritePolicy {
    WRITE_BACK,
    WRITE_THROUGH
};

struct CacheLine {
    uint64_t tag;
    bool valid;
    bool dirty;
    uint32_t lru_counter;
    uint8_t data[64];
};

struct cacheReadResult {
    bool hit;
    float value;
    std::array<uint8_t, 64> linedata;
};

class Cache {
    protected:
        uint32_t cache_size_kb;
        uint32_t cache_line_size_bytes;
        uint32_t num_sets;
        uint32_t associativity;
        uint32_t offset_bits;
        uint32_t index_bits;
        WritePolicy write_policy;

        std::vector<std::vector<CacheLine>> cache;

        uint64_t hits = 0;
        uint64_t misses = 0;
        uint32_t global_lru_counter = 0;

    public:
        Cache(uint32_t cache_size_kb, uint32_t associativity,
            WritePolicy write_policy = WRITE_BACK, uint32_t cache_line_size_bytes = 64) {

            this->cache_size_kb = cache_size_kb;
            this->cache_line_size_bytes = cache_line_size_bytes;
            this->associativity = associativity;
            this->write_policy = write_policy;

            uint32_t cache_size_bytes = cache_size_kb * 1024;
            this->num_sets = cache_size_bytes / (cache_line_size_bytes * associativity);

            cache.resize(num_sets);
            for (int i = 0; i < num_sets; i++) {
                cache[i].resize(associativity);
                for (int j = 0; j < associativity; j++) {
                    cache[i][j].tag = 0;
                    cache[i][j].valid = false;
                    cache[i][j].lru_counter = 0;
                    cache[i][j].dirty = false;
                }
            }
            offset_bits = (uint32_t)log2(cache_line_size_bytes);
            index_bits  = (uint32_t)log2(num_sets);
        }

        uint32_t extract_offset(uint64_t address) { return address & ((1 << offset_bits) - 1); }
        uint32_t extract_index(uint64_t address)  { return (address >> offset_bits) & ((1 << index_bits) - 1); }
        uint64_t extract_tag(uint64_t address)    { return address >> (offset_bits + index_bits); }

        int find_lru_victim(uint32_t set_index) {
            uint32_t min_lru = cache[set_index][0].lru_counter;
            int victim = 0;
            for (int j = 0; j < associativity; j++) {
                if (cache[set_index][j].lru_counter < min_lru) {
                    min_lru = cache[set_index][j].lru_counter;
                    victim = j;
                }
            }
            return victim;
        }

        void set_dirty(uint64_t address) {
            uint32_t index = extract_index(address);
            uint64_t tag   = extract_tag(address);
            for (int j = 0; j < associativity; j++) {
                if (cache[index][j].valid && cache[index][j].tag == tag)
                    cache[index][j].dirty = true;
            }
        }

        cacheReadResult read(uint64_t address) {
            uint32_t index  = extract_index(address);
            uint64_t tag    = extract_tag(address);
            uint32_t offset = extract_offset(address);
            cacheReadResult result;
            for (int j = 0; j < associativity; j++) {
                if (cache[index][j].valid && cache[index][j].tag == tag) {
                    hits++;
                    cache[index][j].lru_counter = global_lru_counter++;
                    result.hit = true;
                    float value;
                    memcpy(&value, &cache[index][j].data[offset], sizeof(float));
                    memcpy(result.linedata.data(), cache[index][j].data, 64);
                    result.value = value;
                    return result;
                }
            }
            misses++;
            result.hit = false;
            return result;
        }

        bool write(uint64_t address, float value) {
            uint32_t index  = extract_index(address);
            uint64_t tag    = extract_tag(address);
            uint32_t offset = extract_offset(address);
            for (int j = 0; j < associativity; j++) {
                if (cache[index][j].valid && cache[index][j].tag == tag) {
                    memcpy(&cache[index][j].data[offset], &value, 4);
                    cache[index][j].lru_counter = global_lru_counter++;
                    return true;
                }
            }
            return false;
        }

        void cacheInstall(uint64_t address, std::array<uint8_t, 64> data, MainMemory& memory) {
            uint32_t index = extract_index(address);
            uint64_t tag   = extract_tag(address);
            for (int j = 0; j < associativity; j++) {
                if (!cache[index][j].valid) {
                    cache[index][j].valid = true;
                    cache[index][j].tag   = tag;
                    cache[index][j].dirty = false;
                    cache[index][j].lru_counter = global_lru_counter++;
                    memcpy(cache[index][j].data, data.data(), 64);
                    return;
                }
            }
            int victim = find_lru_victim(index);
            if (cache[index][victim].dirty && write_policy == WRITE_BACK) {
                uint64_t tgt = ((uint64_t)cache[index][victim].tag << (index_bits + offset_bits))
                             | ((uint64_t)index << offset_bits);
                memory.write_line(cache[index][victim].data, tgt);
            }
            cache[index][victim].tag   = tag;
            cache[index][victim].dirty = false;
            cache[index][victim].valid = true;
            cache[index][victim].lru_counter = global_lru_counter++;
            memcpy(cache[index][victim].data, data.data(), 64);
        }

        void print_stats() {
            uint64_t total = hits + misses;
            double hit_rate  = (hits   / (double)total) * 100.0;
            double miss_rate = (misses / (double)total) * 100.0;
            std::cout << "\n=== Cache Statistics ===" << std::endl;
            std::cout << "Total accesses: " << total  << std::endl;
            std::cout << "Hits: "           << hits   << std::endl;
            std::cout << "Misses: "         << misses << std::endl;
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "Hit rate: "  << hit_rate  << "%" << std::endl;
            std::cout << "Miss rate: " << miss_rate << "%" << std::endl;
        }

        double get_hit_rate() const {
            uint64_t total = hits + misses;
            if (total == 0) return 0.0;
            return (hits / (double)total) * 100.0;
        }

        uint64_t get_hits() const {
            return hits;
        }

        uint64_t get_misses() const {
            return misses;
        }

        uint64_t get_total_accesses() const {
            return hits + misses;
        }

        void reset_stats() {
            hits = 0;
            misses = 0;
            //global_lru_counter = 0;
        }

        void print_occupancy() {
            int valid_lines = 0;
            for (int i = 0; i < num_sets; i++) {
                int valid_in_set = 0;
                for (int j = 0; j < associativity; j++) {
                    if (cache[i][j].valid) valid_in_set++;
                }
                valid_lines += valid_in_set;
                std::cout << "  set " << i << ": " << valid_in_set << "/" << associativity << " valid";
                if (valid_in_set > 0) {
                    std::cout << " | tags: ";
                    for (int j = 0; j < associativity; j++) {
                        if (cache[i][j].valid) std::cout << cache[i][j].tag << " ";
                    }
                }
                std::cout << std::endl;
            }
            std::cout << "  TOTAL: " << valid_lines << "/" << (num_sets * associativity) << " lines valid" << std::endl;
        }
};

class CacheL1 : public Cache {
    public:
        CacheL1(uint32_t cache_size_kb, uint32_t associativity,
                WritePolicy write_policy = WRITE_BACK)
            : Cache(cache_size_kb, associativity, write_policy) {}
};

class CacheL2 : public Cache {
    public:
        CacheL2(uint32_t cache_size_kb, uint32_t associativity,
                WritePolicy write_policy = WRITE_BACK)
            : Cache(cache_size_kb, associativity, write_policy) {}
};

class CacheL3 : public Cache {
    public:
        CacheL3(uint32_t cache_size_kb, uint32_t associativity,
                WritePolicy write_policy = WRITE_BACK)
            : Cache(cache_size_kb, associativity, write_policy) {}
};

#endif