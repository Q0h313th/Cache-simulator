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

        /* Main cache! */
        std::vector<std::vector<CacheLine>> cache;
        
        /* Statistics */
        uint64_t hits = 0;
        uint64_t misses = 0;
        uint32_t global_lru_counter = 0;

    public:
        Cache(uint32_t cache_size_kb, uint32_t associativity, uint32_t cache_line_size_bytes = 64) {
            
            this->cache_size_kb = cache_size_kb;
            this->cache_line_size_bytes = cache_line_size_bytes;
            this->associativity = associativity;

            /* num_sets = (cache_size_kb * 1024) / (line_size * associativty) */
            uint32_t cache_size_bytes = cache_size_kb * 1024;
            this->num_sets = cache_size_bytes / (cache_line_size_bytes * associativity);

            /* 
            set associative caches are 2D matrices of the sort cache[num_sets][associativity]
            */
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
            index_bits = (uint32_t)log2(num_sets);
        }

        /* last offset_bits number of bits */
        uint32_t extract_offset(uint64_t address) {
            return address & ((1 << offset_bits) - 1);
        }
        /* the next index_bits number of bits */
        uint32_t extract_index(uint64_t address) {
            return (address >> offset_bits) & ((1 << index_bits) - 1);
        }
        /* the remaining number of bits */
        uint64_t extract_tag(uint64_t address) {
            return address >> (offset_bits + index_bits);
        }

        int find_lru_victim(uint32_t set_index) {
            uint32_t min_lru_count = cache[set_index][0].lru_counter;
            int victim_way = 0;
            for (int j = 0; j < associativity; j++) {
                if (cache[set_index][j].lru_counter < min_lru_count) {
                    min_lru_count = cache[set_index][j].lru_counter;
                    victim_way = j;
                }
            }
            return victim_way;
        }

        /* cache.read() */
        cacheReadResult read(uint64_t address) {

            uint32_t index = extract_index(address);
            uint64_t tag = extract_tag(address);
            uint32_t offset = extract_offset(address);
            float value;
            std::array<uint8_t, 64> linedata;
            cacheReadResult result;

            /* search for matching tag in cache[set][j] */
            for (int j = 0; j < associativity; j++) {
                if (cache[index][j].valid && cache[index][j].tag == tag) {
                    hits++;
                    cache[index][j].lru_counter = global_lru_counter;
                    global_lru_counter++;
                    result.hit = true;
                    memcpy(&value, &cache[index][j].data[offset], sizeof(float));
                    memcpy(linedata.data(), &cache[index][j].data, 64);
                    result.value = value;
                    result.linedata = linedata;
                    return result;
                }
            }
            /* cache miss! :( */
            misses++;
            result.hit = false;
            return result;
        }

        void write(uint64_t address, float value) {
            uint32_t index = extract_index(address);
            uint64_t tag = extract_tag(address);
            uint32_t offset = extract_offset(address);

            for (int j = 0; j < associativity; j++) {
                if (cache[index][j].valid && cache[index][j].tag == tag) {
                    memcpy(&cache[index][j].data[offset], &value, 4);
                }
            }
        }

        /* cache.install() */
        void cacheInstall(uint64_t address, std::array<uint8_t, 64> data) {

            uint32_t index = extract_index(address);
            uint64_t tag = extract_tag(address);
            for (int j = 0; j < associativity; j++) {
                if (!cache[index][j].valid) {
                    cache[index][j].valid = true;
                    cache[index][j].tag = tag;
                    cache[index][j].lru_counter = global_lru_counter;
                    global_lru_counter++;
                    memcpy(cache[index][j].data, data.data(), 64); 
                    return;
                }
            }
            // case for if the cache is full and we need to evict
            // we need to writeback/writethrough here depending on the policies, but for now we just overwrite
            int victim_way = find_lru_victim(index);
            cache[index][victim_way].tag = tag;
            cache[index][victim_way].lru_counter = global_lru_counter;
            global_lru_counter++;
            memcpy(cache[index][victim_way].data, data.data(), 64);
        }

        void print_stats(){
            uint64_t total = hits + misses;
            double hit_rate = (hits / (double)total) * 100.0;
            double miss_rate = (misses / (double)total) * 100.0;
            
            std::cout << "\n=== Cache Statistics ===" << std::endl;
            std::cout << "Total accesses: " << total << std::endl;
            std::cout << "Hits: " << hits << std::endl;
            std::cout << "Misses: " << misses << std::endl;
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "Hit rate: " << hit_rate << "%" << std::endl;
            std::cout << "Miss rate: " << miss_rate << "%" << std::endl;
        }
};

class CacheL1: public Cache {
    public:
        CacheL1(uint32_t cache_size_kb, uint32_t associativity): Cache(cache_size_kb, associativity) {}
};

class CacheL2: public Cache {
    public:
        CacheL2(uint32_t four_times_cache_size_kb, uint32_t associativity): Cache(four_times_cache_size_kb, associativity) {}
};

class CacheL3: public Cache{
    public:
        CacheL3(uint32_t sixteen_times_cache_size_kb, uint32_t associativity): Cache(sixteen_times_cache_size_kb, associativity) {}
};

#endif