#include "cache.h"
#include "MainMemory.h"
#include "memory_hierarchy.h"

int main() {
    MemoryHierarchy mem(4, 4, 8, 16);
    Cache cache(32, 12);
    return 0;
}