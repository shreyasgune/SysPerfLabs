# linux-perf


These are some examples of using the perf Linux profiler, which has also been called Performance Counters for Linux (PCL), Linux perf events (LPE), or perf_events. 

perf_events is part of the Linux kernel, under tools/perf. While it uses many Linux tracing features, some are not yet exposed via the perf command, and need to be used via the ftrace interface instead.

With this effort, I'm trying to answer:
    Why is the kernel on-CPU so much? What code-paths?
    Which code-paths are causing CPU level 2 cache misses?
    Are the CPUs stalled on memory I/O?
    Which code-paths are allocating memory, and how much?
    What is triggering TCP retransmits?
    Is a certain kernel function being called, and how often?
    What reasons are threads leaving the CPU?

## Compile and Run genLoad
`g++ -o genLoad genLoad.cpp -std=c++17`


`perf record -g ./genLoad 10` generate load for 10 seconds

## Results




## Gotchas

If you hit into an issue where perf is not installed on your system, you might need to update your kernel

```
sudo apt-get update && sudo apt-get dist-upgrade


```