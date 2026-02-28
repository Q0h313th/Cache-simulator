#ifndef MAINMEMORY_H
#define MAINMEMORY_H

#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <cassert>
#include <array>
#include <cstring>

class MainMemory {
    private:
        std::unordered_map<uint64_t, float> memory;

    // we dont need any constructor because unordered_map grows dynamically
    public:
        float read_float(uint64_t address) {
            assert(memory.find(address) != memory.end() && "Invalid read! failure at MainMemory::read_float");
            return memory.at(address);
        }

        void write_float(uint64_t address, float value) {
            memory[address] = value; // so we can overwrite memory freely
        }

        std::array<uint8_t, 64> read_line(uint64_t address) {
            std::array<uint8_t, 64> block;
            uint64_t block_addr = address & (~63ULL);
            for (int i = 0; i < 16; i++) {
                float value = read_float(block_addr + (i*4));
                memcpy(&block[i*4], &value, 4);
            }
            return block;
        }

};

#endif