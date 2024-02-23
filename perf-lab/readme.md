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


- counting events in-kernel context, where a summary of counts is printed by perf. This mode does not generate a perf.data file.
- sampling events, which writes event data to a kernel buffer, which is read at a gentle asynchronous rate by the perf command to write to the perf.data file. This file is then read by the perf report or perf script commands.
- bpf programs on events, a new feature in Linux 4.4+ kernels that can execute custom user-defined programs in kernel space, which can perform efficient filters and summaries of the data. Eg, efficiently-measured latency histograms.


## Compile and Run genLoad
`g++ -o genLoad genLoad.cpp -std=c++17`


`perf record -g ./genLoad 10` generate load for 10 seconds

## Results
```
# perf record -g ./genLoad 10
Generating load for 10 seconds...
Load Gen complete
[ perf record: Woken up 3 times to write data ]
[ perf record: Captured and wrote 0.614 MB perf.data (10010 samples) ]

// to see the report results
# perf report



//perf stat on running genLoad
# perf stat ./genLoad 10
Generating load for 10 seconds...
Load Gen complete

 Performance counter stats for './genLoad 10':
 // these stats are from CPU's Performance Monitoring Units (PMU).

          10015.57 msec task-clock                       #    1.000 CPUs utilized
                41      context-switches                 #    4.094 /sec
                 2      cpu-migrations                   #    0.200 /sec
               389      page-faults                      #   38.840 /sec
   <not supported>      cycles // number of CPU cycles elapsed
   <not supported>      instructions // number of instruction executed.
   <not supported>      branches // number of branch instructions executed
   <not supported>      branch-misses // number of misinterpreted branches. This happens when processor tries to predict (to make prefetch instructions) the outcome of a branch incorrectly

    // if instruction count is high but cycles is low - code execution is efficient
    // high cache miss rates means cache is being utilized poorly
    // high branch misses means inefficient branching in your code. Branches are if conditionals, function calls, jump statements etc.
    // minimizing branching complexity makes things a bit better cuz lesser the branches, lesser the chance of branch-misses
    // we can generally interpret high IPC values (eg, over 1.0) as good, indicating optimal processing of work. However, also double check what the instructions are, 
       in case this is due to a spin loop: a high rate of instructions, but a low rate of actual work completed.


      10.017952671 seconds time elapsed

      10.012545000 seconds user
       0.003999000 seconds sys

```

Collected system and performance data may be split into several categories:

- System hardware and software configuration data, for example: a CPU model and its cache configuration, an amount of available memory and its topology, used kernel and Perf versions, performance monitoring setup including experiment time, events configuration, Perf command line parameters, etc.

- User and kernel module paths and their load addresses with sizes, process and thread names with their PIDs and TIDs, timestamps for captured hardware and software events.

- Content of kernel software counters (e.g., for context switches, page faults, CPU migrations), architectural hardware performance counters (PMC) and machine specific registers (MSR) that provide execution metrics for various monitored parts of the system (e.g., memory controller (IMC), interconnect (QPI/UPI) or peripheral (PCIe) uncore counters) without direct attribution to any execution context state.

- Content of architectural execution context registers (e.g., RIP, RSP, RBP on x86_64), process user and kernel space memory addresses and data, content of various architectural MSRs that capture data from this category.




## Gotchas

If you hit into an issue where perf is not installed on your system, you might need to update your kernel

```
$ sudo apt-get update && sudo apt-get dist-upgrade

$ sudo apt-cache search linux-tools
linux-perf - Performance analysis tools for Linux

$ sudo apt-get install -y linux-perf

$ perf --version
perf version 6.1.76

# perf stat
Error:
Access to performance monitoring and observability operations is limited.
Consider adjusting /proc/sys/kernel/perf_event_paranoid setting to open
access to performance monitoring and observability operations for processes
without CAP_PERFMON, CAP_SYS_PTRACE or CAP_SYS_ADMIN Linux capability.
More information can be found at 'Perf events and tool security' document:
https://www.kernel.org/doc/html/latest/admin-guide/perf-security.html
perf_event_paranoid setting is 2:
  -1: Allow use of (almost) all events by all users
      Ignore mlock limit after perf_event_mlock_kb without CAP_IPC_LOCK
>= 0: Disallow raw and ftrace function tracepoint access
>= 1: Disallow CPU event access
>= 2: Disallow kernel profiling
To make the adjusted perf_event_paranoid setting permanent preserve it
in /etc/sysctl.conf (e.g. kernel.perf_event_paranoid = <setting>)

```