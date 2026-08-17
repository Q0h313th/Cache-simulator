Fair criticism.

Looking back at the evidence we actually had, there was already enough to form a working hypothesis instead of continuing to pile on diagnostics.

The strongest facts were:

before eviction = 123
after eviction = 0
Dirty evictions were occurring.
The hierarchy installs the same line into L1/L2/L3 on a miss.
Writes only modify L1.

That is already sufficient justification to redesign the writeback path, regardless of whether the final bug is only coherence or coherence plus something else.

If I were continuing myself, I'd stop debugging and do exactly what you proposed earlier:

cacheInstall()
    ↓
returns EvictionInfo

MemoryHierarchy
    ↓
handles propagation

L1 eviction → L2
L2 eviction → L3
L3 eviction → Memory

because that's architecturally correct even if it doesn't immediately fix every symptom.

One additional thing I'd keep in the back of my mind when you return to it:

Your hierarchy currently behaves more like an inclusive cache hierarchy because you install lines into all levels simultaneously. In an inclusive hierarchy, lower levels must be kept coherent with upper levels. The current design violates that invariant the moment an L1 write occurs. So even without the debugging output, there is a real design flaw that needs fixing.

Good luck with it. Sometimes it's faster to step away from the instrumentation spiral and just implement the cleaner architecture. Once the hierarchy has a proper ownership/writeback model, any remaining bugs tend to become much easier to reason about.