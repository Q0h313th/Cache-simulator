#ifndef MAINMEMORY_H
#define MAINMEMORY_H

#include <iostream>
#include <string>
#include <cstdint>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <unordered_map>
#include <cassert>

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

};

#endif