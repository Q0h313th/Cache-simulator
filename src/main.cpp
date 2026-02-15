#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <sstream>
#include <vector>

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

        /* Main cache! */
        std::vector<std::vector<CacheLine>> cache;
        
        /* Statistics */
        uint64_t hits;
        uint64_t misses;
        uint32_t global_lru_counter;

        /* Helper methods */
        uint32_t extract_offset(uint64_t address);
        uint32_t extract_index(uint64_t address);
        uint64_t extract_tag(uint64_t address);
        int find_lru_victim(uint32_t set_index);

    public:
        Cache(uint32_t cache_size_kb, uint32_t cache_line_size_bytes, uint32_t associativity) {
            /* Calculate the number of sets? */
        }

        /* Perform the access */
        bool access(uint64_t address) {

        }

        /* Print statistics */
        void print_stats(){

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
    std::string FileName = "test1.txt";
    std::ifstream file(FileName);
    
    if (!file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }
    
    std::string line;
    while(std::getline(file, line)) {
        MemoryAccess access = parseTraceLine(line);
        
        if (access.event != '\0') {
            // std::cout << access.event << ',' << std::hex << access.address << ',' << std::dec << access.size << std::endl;
            
        } 
    }
    file.close();
    return 0;
}