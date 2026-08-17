# Cache Hierarchy Experiments — Project Brief

## Context

This is an educational/demonstrative project built around a simulated multi-level
cache hierarchy in C++. The goal is to run a series of experiments that make
core cache behavior (capacity, associativity, conflict misses, write policy,
access patterns) *visible* through hit-rate and traffic measurements, rather
than just stating the theory.

## Existing codebase

The simulator already implements:

- **`MainMemory.h`** — a simple backing store keyed by address, with
  `read_line` / `write_line` operating on 64-byte cache lines.
- **`cache.h`** — a set-associative `Cache` base class (parameterized by size,
  associativity, line size, write policy) with LRU replacement, plus thin
  `CacheL1` / `CacheL2` / `CacheL3` subclasses. Tracks hits/misses and can
  report hit rate.
- **`memory_hierarchy.h`** — a `MemoryHierarchy` class wiring L1 → L2 → L3 →
  main memory together, handling inclusion/install-on-miss and write-allocate
  behavior for both `WRITE_BACK` and `WRITE_THROUGH` policies.
- **`strided_access.cpp`** — a first experiment: reads through memory with
  varying strides (1, 2, 4, 8, 16, 32, 64) at a fixed access count, printing
  per-level hit rates for each stride.

The simulator is address-driven (not tied to real allocated memory), so any
experiment just needs to generate a sequence of addresses (and optionally
values to write) and feed them to `MemoryHierarchy::read_mem` /
`write_mem`, then read off hit rates via `print_stats()` /
`print_stats_oneline()` or `Cache::get_hit_rate()`.

## Goal

Design and implement a set of small, focused experiments — each isolating
one architectural concept — that produce clear, plottable/printable results
suitable for teaching cache behavior. Each experiment should:

1. Use the existing `MemoryHierarchy` / `Cache` / `MainMemory` API without
   needing structural changes (or with minimal, clearly justified additions).
2. Vary exactly one independent variable at a time where possible, to keep
   the causal story clean.
3. Produce a simple, interpretable output (hit rate vs. parameter, ideally
   easy to pipe into a CSV or plot).
4. Come with a short written explanation of *why* the result looks the way
   it does, tying it back to the underlying mechanism (capacity, mapping
   function, associativity, replacement policy, write policy, spatial vs.
   temporal locality).

## Planned experiments

### 1. Memory mountain (working-set size sweep)
Fix a sequential (stride-1) access pattern and sweep the *size* of the
working set from well below L1 capacity up past L3 capacity. Plot hit rate
(or effective access latency, if we add per-level latency weights) against
working-set size. Expect clean step-downs at each cache-capacity boundary.

### 2. Conflict misses via power-of-two aliasing
Pick a working set that fits well within a cache's *capacity*, but choose a
stride that is a multiple of the cache's number of sets (or a large power of
two), so most accesses map to the same set. Show the hit rate collapse
despite the small working set — an explicit illustration of conflict misses
as distinct from capacity misses.

### 3. Associativity comparison
Using the conflict-prone access pattern from (2), sweep associativity (1, 2,
4, 8, fully associative if feasible) while holding cache size and access
pattern fixed. Show hit rate recovering as associativity increases, with
diminishing returns past some point.

### 4. Matrix traversal: row-major vs. column-major, with/without tiling
Simulate access to a 2D matrix stored in row-major order. Compare:
- naive `A[i][j]` (row-major, stride-1 within a row) vs.
- `A[j][i]` (column-major traversal, large stride = row width)
- a blocked/tiled traversal that restores locality.
Show hit-rate differences and connect to real-world loop-order/tiling
optimization advice.

### 5. Write policy comparison (write-back vs. write-through)
Instrument `MainMemory` (or wrap it) to count writes/writebacks. Run a
write-heavy workload under `WRITE_BACK` and `WRITE_THROUGH` and compare
total memory traffic and dirty-line writeback counts. Illustrate the
traffic/complexity tradeoff between the two policies.

### 6. Pointer chasing vs. array streaming
Compare a workload that walks a randomly-scattered linked structure
(dependent, unpredictable addresses) against a workload that streams
sequentially through an array of equivalent size. Highlights that hit rate
alone doesn't capture everything — this experiment motivates
latency/throughput distinctions and prefetch-friendliness, complementing the
purely hit-rate-based experiments above.

## Suggested order of implementation

1. Memory mountain (reuses almost all existing code — just parameterize
   working-set size instead of stride).
2. Matrix traversal (most intuitive "why this matters for real code" case).
3. Conflict misses / associativity sweep (deeper, more mechanism-focused).
4. Write policy comparison.
5. Pointer chasing vs. streaming.

## Open questions / design decisions to make while implementing

- Should experiments report hit rate only, or also model relative latency
  per level (e.g. L1=1 cycle, L2=10, L3=40, memory=200) to produce an
  "effective access time" metric that's more intuitive than raw hit rate?
- Do we want CSV output from each experiment for easy plotting, or is
  console output sufficient for now?
- Should write-heavy experiments extend `MainMemory` with a write counter,
  or should this be tracked at the `Cache` level via a writeback counter?
- For the pointer-chasing experiment, do we need an actual linked-list
  address generator, or can we simulate "randomness" via a fixed
  pseudo-random permutation of addresses for reproducibility?