#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <sstream>
#include <vector>
#include <cmath>
#include <iomanip>

struct CacheLine {
    uint64_t tag;
    bool valid;
    uint32_t lru_counter;
};

struct MemoryAccess {
    char event;
    uint64_t address;
    uint64_t size;
};

/* Define the Cache structure class */
class Cache {
    private:
        uint32_t cache_size_kb;
        uint32_t cache_line_size_bytes;
        uint32_t num_sets;
        uint32_t associativity;
        uint32_t offset_bits;
        uint32_t index_bits;
        uint32_t tag_bits;

        /* Main cache! */
        std::vector<std::vector<CacheLine>> cache;
        
        /* Statistics */
        uint64_t hits = 0;
        uint64_t misses = 0;
        uint32_t global_lru_counter = 0;

    public:
        Cache(uint32_t cache_size_kb, uint32_t cache_line_size_bytes, uint32_t associativity) {
            /* Calculate the number of sets? */
            this->cache_size_kb = cache_size_kb;
            this->cache_line_size_bytes = cache_line_size_bytes;
            this->associativity = associativity;

            /* num_sets = (cache_size_kb * 1024) / (line_size * associativty) */
            uint32_t cache_size_bytes = cache_size_kb * 1024;
            this->num_sets = cache_size_bytes / (cache_line_size_bytes * associativity);

            /* 
            set associative caches are 2D matrices of the sort cache[num_sets][associativity]
            so, the search would essentially be: 
            1. Which set? (give me the row number)
            2. Whats the tag? Since every address has a different tag, we search each column (while (i < associativity)) until we get the tag
            3. If we didnt find the tag, then we should insert this address in the cache. If the cache is full, then we eject a victim line and insert our new address there. 
            */
            cache.resize(num_sets);
            for (int i = 0; i < num_sets; i++) {
                cache[i].resize(associativity);
                for (int j = 0; j < associativity; j++) {
                    cache[i][j].tag = 0;
                    cache[i][j].valid = false;
                    cache[i][j].lru_counter = 0;
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

        /* Perform the access */
        bool access(uint64_t address) {
            uint32_t index = extract_index(address);
            uint64_t tag = extract_tag(address);

            /* search for matching tag in cache[set][j] */
            for (int j = 0; j < associativity; j++) {
                if (cache[index][j].valid && cache[index][j].tag == tag) {
                    hits++;
                    cache[index][j].lru_counter = global_lru_counter;
                    global_lru_counter++;
                    return true;
                }
            }
            /* cache miss! :( */
            misses++;
            /* check if there are any empty ways*/
            for (int j = 0; j < associativity; j++) {
                if (!cache[index][j].valid) {
                    // insert into this way
                    cache[index][j].tag = tag;
                    cache[index][j].valid = true;
                    cache[index][j].lru_counter = global_lru_counter;
                    global_lru_counter++;
                }
            }
            /* no empty found, so evict LRU way */
            int victim_way = find_lru_victim(index);
            cache[index][victim_way].tag = tag;
            cache[index][victim_way].valid = true;
            cache[index][victim_way].lru_counter = global_lru_counter;
            global_lru_counter++;

            return false;
        }

        /* Print statistics */
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

/* Parse the trace line and store the values into a struct */
MemoryAccess parseTraceLine(std::string Line) {
    struct MemoryAccess result;
    result.event = '\0';
    
    std::istringstream iss(Line);
    char comma;
    char event;
    uint64_t address;
    uint64_t size;
    
    if (iss >> event >> std::hex >> address >> comma >> size) {
        result.event = event;
        result.address = address;
        result.size = size;
    }
    
    return result;
}

int main(){
    std::string FileName = "../tests/test1.txt";
    std::ifstream file(FileName);
    
    if (!file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }
    
    Cache cache(64, 64, 12);
    
    std::string line;
    int access_count = 0;
    while(std::getline(file, line)) {
        MemoryAccess access = parseTraceLine(line);
        
        if (access.event != '\0') {
            bool hit = cache.access(access.address);
            
            // Print each access
            access_count++;
            std::cout << access_count << ": " 
                      << access.event << " "
                      << std::hex << access.address << std::dec << " "
                      << "-> " << (hit ? "HIT" : "MISS") << std::endl;
        } 
    }

    file.close();
    cache.print_stats();
    return 0;
}