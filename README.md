# blockcacheperf

This is a framework for comparing the performance of various block cache
eviction algorithms.

It can capture application disk access traces, simulate eviction algorithms
for given traces, and replay the post-cache output of those algorithms on
raw disk device to get a more realistic measure of their performance.

2Q, ARC, BRRIP, CAR, CLOCK, CLOCKPRO, FAB, LRU, Random2, RRIP, SpatialClock
algorithms are supported.

This project is developed as part of my Master Thesis at Harvard University
Extension School. You can read the paper from [here](doc/MasterThesis.pdf).
