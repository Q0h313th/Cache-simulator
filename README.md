# Cache Simulator

A C++ simulator for a three-level (L1 → L2 → L3 → main memory) set-associative
cache hierarchy, built to make core cache behavior — capacity, associativity,
conflict misses, write policy, and access-pattern effects — visible through
hit-rate measurements rather than just theory.

The simulator is **address-driven**: it doesn't operate on real allocated
memory. You feed it a sequence of addresses (and optionally values) through
`read_mem` / `write_mem`, and it tracks hits/misses at every level.

## Architecture

```
MemoryHierarchy
 ├── CacheL1 ─┐
 ├── CacheL2 ─┼── all inherit from Cache (set-associative, LRU replacement)
 ├── CacheL3 ─┘
 └── MainMemory (backing store)
```

### `MainMemory.h`
A simple backing store keyed by address (`std::unordered_map<uint64_t, float>`).
Reads/writes work at the granularity of a single `float`, but also exposes
`read_line` / `write_line` for moving whole 64-byte cache lines in and out of
memory. Any address not yet written reads back as `0.0f`.

### `cache.h`
Defines the `Cache` base class and the `CacheL1` / `CacheL2` / `CacheL3`
subclasses (currently thin wrappers with no added behavior).

Each `Cache` is parameterized by:
- `cache_size_kb` — total cache size
- `associativity` — ways per set
- `write_policy` — `WRITE_BACK` or `WRITE_THROUGH`
- `cache_line_size_bytes` — defaults to 64 bytes

Internally it's a `std::vector<std::vector<CacheLine>>` (sets × ways), with
each `CacheLine` holding a tag, valid/dirty bits, an LRU counter, and 64 bytes
of data. Address bits are split into offset / index / tag based on line size
and number of sets.

Key operations:
- **`read(address)`** — returns a `cacheReadResult` (`hit`, extracted `value`,
  and the full 64-byte `linedata` for installing into higher levels on a miss).
- **`write(address, value)`** — writes in place on a hit; returns `false` on
  a miss (the caller is responsible for allocate-on-write-miss handling).
- **`cacheInstall(address, data, memory)`** — installs a line into a free way,
  or evicts the LRU victim, writing it back to `MainMemory` first if it's
  dirty and the cache uses `WRITE_BACK`.
- **`set_dirty(address)`**, **`get_hit_rate()`**, **`print_stats()`**,
  **`print_occupancy()`**, **`reset_stats()`** — bookkeeping/reporting.

Replacement policy is LRU, tracked via a monotonically increasing
`global_lru_counter` stamped on every access/install.

### `memory_hierarchy.h`
Wires `CacheL1 → CacheL2 → CacheL3 → MainMemory` together:

- **`read_mem(address)`** — checks L1, then L2, then L3, then main memory,
  installing the line into every level above the one that hit (or above
  main memory on a full miss). This models an *inclusive*, install-on-miss
  hierarchy.
- **`write_mem(address, value)`** — behavior depends on `write_policy`:
  - `WRITE_BACK` (write-allocate): on an L1 hit, writes in place and marks
    the line dirty. On a miss, the line is fetched from `MainMemory`, the
    new value is patched into the block *before* installing it into
    L1/L2/L3, and the L1 copy is marked dirty. Dirty data is only written
    back to `MainMemory` on eviction.
  - `WRITE_THROUGH`: writes straight to `MainMemory` and also updates L1
    if present, on every write.
- **`print_stats()` / `print_stats_oneline()` / `print_hits()` /
  `print_occupancy()` / `reset_stats()`** — reporting helpers that
  aggregate across all three cache levels.

In the current constructor, L2 and L3 sizes are derived from L1 size:
`L2 = 4 × L1`, `L3 = 16 × L1` (each with its own associativity).

## Building

The project has no build system yet — compile each experiment directly
against the headers, e.g.:

```bash
g++ -O2 -std=c++17 exp1_memory_mountain.cpp -o memory_mountain
./memory_mountain
```

## Experiments

### 1. Memory mountain (`exp1_memory_mountain.cpp`)

Sweeps the working-set size across a fixed stride-1 access pattern, from well
below L1 capacity to well past L3 capacity, and reports per-level hit rate
after 10 repeated passes over the same footprint.

```cpp
MemoryHierarchy mem(4, 4, 8, 16);   // L1 = 4KB, L2 = 16KB, L3 = 64KB
```

Working-set sizes tested: `1, 2, 4, 8, 16, 32, 64, 128` KB.

Each run:
1. Writes the full working set once (cold fill), *before* resetting stats,
   so the initial write-miss traffic doesn't pollute the read hit-rate
   numbers.
2. Resets stats.
3. Performs `num_passes = 10` full read sweeps over the working set and
   reports the aggregate per-level hit rate.

#### Results

```
working set =     1 KB | L1: 100.00%  L2: 0.00%    L3: 0.00%
working set =     2 KB | L1: 100.00%  L2: 0.00%    L3: 0.00%
working set =     4 KB | L1: 100.00%  L2: 0.00%    L3: 0.00%
working set =     8 KB | L1: 93.75%   L2: 100.00%  L3: 0.00%
working set =    16 KB | L1: 93.75%   L2: 100.00%  L3: 0.00%
working set =    32 KB | L1: 93.75%   L2: 0.00%    L3: 100.00%
working set =    64 KB | L1: 93.75%   L2: 0.00%    L3: 100.00%
working set =   128 KB | L1: 93.75%   L2: 0.00%    L3: 0.00%
```

#### Reading the results

- **≤ 4 KB (fits in L1):** 100% L1 hit rate — the whole working set stays
  resident, so after the cold fill every subsequent pass is pure hits.
- **8–16 KB (exceeds L1, fits in L2):** L1 hit rate drops to 93.75% and L2
  jumps to 100%. This matches the expected step: the working set no longer
  fits in the 4 KB L1, so each pass now overflows into L2, but 8–16 KB still
  fits comfortably inside the 16 KB L2.
- **32–64 KB (exceeds L2, fits in L3):** L2 hit rate drops to 0% and L3 rises
  to 100%, while L1 stays at 93.75% — consistent with the working set now
  overflowing L2 entirely but still fitting inside the 64 KB L3.
- **128 KB (exceeds L3):** L3 hit rate drops to 0%, as expected once the
  working set exceeds all three cache capacities. L1 unexpectedly still
  reads 93.75% here — see note below.

#### Anomalies worth investigating

Two numbers in this run don't fully match the "clean staircase" story the
plan describes, and are worth digging into before trusting the mountain plot
for teaching purposes:

1. **L1 hit rate is flat at 93.75% for every working-set size ≥ 8 KB**,
   instead of continuing to *decrease* as the working set grows well past
   L1's capacity (e.g. at 128 KB, you'd expect L1 hit rate to be close to
   0%, not still 93.75%). A flat 93.75% = 15/16 suggests something size-
   independent is going on — worth checking whether this is a spatial-
   locality artifact of `cacheInstall` (e.g. one line reused per 16 line-
   installs due to how addresses map to lines) rather than a genuine
   capacity effect.
2. **At 128 KB, L2 and L3 are both 0%.** L2 correctly misses (working set
   exceeds L2's 16 KB capacity), but L3 should still hit close to 100% once
   installed, similar to the 32–64 KB rows, since 128 KB only just exceeds
   L3's 64 KB capacity via associativity/mapping — a 0% L3 hit rate here is
   an outlier compared to the otherwise-clean pattern and suggests a mapping
   or conflict-miss effect specific to this working-set size, not simple
   capacity overflow.

Both are good candidates for a follow-up experiment (e.g. logging per-pass
hit rate instead of an aggregate across all 10 passes, or printing
`print_occupancy()` after the fill) before drawing firm conclusions from the
mountain shape.

## Planned experiments (not yet implemented)

Per `plan.md`, the memory mountain (sweeping working-set size at stride 1)
is the first of a planned series. Natural next steps in the same spirit —
isolating one architectural variable at a time — include:

- **Associativity sweep** — fix working-set size and stride, vary
  associativity, to show conflict misses shrinking as associativity
  increases.
- **Stride sweep** — vary access stride (already partially explored in
  `strided_access.cpp`) to show the effect of spatial locality on hit rate.
- **Write policy comparison** — compare `WRITE_BACK` vs `WRITE_THROUGH`
  traffic to main memory under write-heavy workloads.

## File overview

| File | Purpose |
|---|---|
| `MainMemory.h` | Backing store, addressed by 64-byte lines |
| `cache.h` | Set-associative cache w/ LRU (`Cache`, `CacheL1/2/3`) |
| `memory_hierarchy.h` | Wires L1→L2→L3→memory, read/write orchestration |
| `strided_access.cpp` | Experiment: hit rate vs. access stride |
| `exp1_memory_mountain.cpp` | Experiment: hit rate vs. working-set size |
| `plan.md` | Project brief and experiment roadmap |