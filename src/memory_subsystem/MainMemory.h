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

    public:
        float read_float(uint64_t address) {
            auto it = memory.find(address);
            if (it != memory.end()) return it->second;
            return 0.0f;
        }

        void write_float(uint64_t address, float value) {
            memory[address] = value;
        }

        std::array<uint8_t, 64> read_line(uint64_t address) {
            std::array<uint8_t, 64> block;
            uint64_t block_addr = address & (~63ULL);
            for (int i = 0; i < 16; i++) {
                float value = read_float(block_addr + (i * 4));
                memcpy(&block[i * 4], &value, 4);
            }
            return block;
        }

        bool write_line(uint8_t data[64], uint64_t address) {
            uint64_t block_addr = address & (~63ULL);
            float value;
            for (int i = 0; i < 16; i++) {
                memcpy(&value, &data[i * 4], 4);
                write_float(block_addr + i * 4, value);
            }
            return true;
        }
};

#endif