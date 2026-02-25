# Impact of compiler optimizations on cache access timing measurements

## Cache Configuration

Reference: 1 KiB = 1024 Bytes

Find cache sizes: `lscpu | grep cache`
Find associativity: `getconf -a | grep -i cache`

### Associativity Values
```
LEVEL1_DCACHE_ASSOC                12
LEVEL2_CACHE_ASSOC                 20
LEVEL3_CACHE_ASSOC                 8
```

### Cache Specifications

**L1 Instruction Cache (L1i)**
- Capacity: 192 KiB = 196,608 bytes
- Line size: 64 bytes
- Associativity: (shared across all cores)

**L1 Data Cache (L1d)**
- Capacity: 288 KiB = 294,912 bytes (49 KB per core)
- Line size: 64 bytes
- Associativity: 12-way
- Number of sets: 294,912 / (64 × 12) = **384 sets**

**L2 Cache**
- Capacity: 1280 KiB = 1,310,720 bytes
- Line size: 64 bytes
- Associativity: 20-way
- Number of sets: 1,310,720 / (64 × 20) = **1024 sets**

**L3 Cache**
- Capacity: 12 MiB = 12,582,912 bytes
- Line size: 64 bytes
- Associativity: 8-way
- Number of sets: 12,582,912 / (64 × 8) = **24,576 sets**

### Cache Summary Table

| Cache | Capacity | Associativity | Line Size | Sets |
|-------|----------|---------------|-----------|------|
| L1d | 288 KiB | 12-way | 64B | 384 |
| L1i | 192 KiB | - | 64B | - |
| L2 | 1280 KiB | 20-way | 64B | 1024 |
| L3 | 12 MiB | 8-way | 64B | 24,576 |

## Address Decoding

For a set-associative cache, the memory address breaks down as:

```
Address: [     TAG (high bits)     ] [ INDEX (mid bits) ] [ OFFSET (low bits) ]
```

- **OFFSET bits** = log₂(line_size) = log₂(64) = 6 bits
- **INDEX bits** = log₂(number_of_sets)
- **TAG bits** = remaining bits

Set index is calculated as: `Set_index = (address >> offset_bits) & ((1 << index_bits) - 1)`

## Experimental Setup

### Thread Pinning

To ensure consistent measurements and avoid thread migration, pin the process to a specific core using `taskset`:

```bash
taskset -c 2 ./<binary>
```

This pins the thread to core 2, keeping measurements consistent across runs.

### Array Size

For these experiments, array size is **10 KB**. This fits entirely in L1d cache (288 KiB).

## Next Steps

Document the specific experiments and their results here.