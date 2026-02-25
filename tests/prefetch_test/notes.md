# Cache timing measurement experiments

## Benchmarking note:
For this experiment, I classify any timing measurement above 1000 cycles to be that of a cold DRAM access (cache miss), and any measurement below 1000 cycles, as a cache hit. I did not attempt to distinguish between L1, L2 and L3 caches since modern cache implementations and other overhead (fences, flushes, other processes) involve complex underlying mechanisms that make it hard to pinpoint which level is being hit. The key distinction here is a simple cache hit vs cache miss.

# Demo 1
- This program is meant to observe the effects as to how changing the stride to access an array changes the access times of each elements.
We have an array `arr[]` the size of 10 pages. We iterate through it with a stride of 0x1000. 
- The fences must be delicately and deliberately placed, otherwise they may get counted as part of the timing data.

