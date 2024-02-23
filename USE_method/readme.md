# Implementing USE method
A method for identifying performance bottlenecks.

- U: Utilization
- S: Saturation
- E: Erorrs

for 
- CPU
- Memory
- IO
- etc..


1. `uptime`
```
root@15fad6dd8666:/SysPerfLabs/USE_method# uptime
 18:45:14 up  1:54,  0 user,  load average: 2.73, 2.56, 2.54
```
shows us the processes wanting to run on CPU, as well as processes blocked in uninterruptible I/O (usually disk I/O). This gives a high level idea of resource load (or demand).

For the above example, a minute ago, there was a relatively high load, and 5 mins & 15 mins ago, the load was in a similar range.

2. `dmesg | tail`
```
root@15fad6dd8666:/SysPerfLabs/USE_method# dmesg | tail
[ 3185.602298] veth602dfee (unregistering): left allmulticast mode
[ 3185.602304] veth602dfee (unregistering): left promiscuous mode
[ 3185.602306] docker0: port 1(veth602dfee) entered disabled state
[ 3209.039401] docker0: port 1(vethdf5c504) entered blocking state
[ 3209.039407] docker0: port 1(vethdf5c504) entered disabled state
[ 3209.039413] vethdf5c504: entered allmulticast mode
[ 3209.039438] vethdf5c504: entered promiscuous mode
[ 3209.164853] eth0: renamed from veth352b131
[ 3209.169878] docker0: port 1(vethdf5c504) entered blocking state
[ 3209.169881] docker0: port 1(vethdf5c504) entered forwarding state
```
This views the last 10 system messages, if there are any. Look for errors that can cause performance issues.

3. `vmstat <time>`
```
root@15fad6dd8666:/SysPerfLabs/USE_method# vmstat 1
procs -----------memory---------- ---swap-- -----io---- -system-- ------cpu-----
 r  b   swpd   free   buff  cache   si   so    bi    bo   in   cs us sy id wa st
 1  0      0 4271324 275588 2893704    0    0    19    55  131  181  0  0 99  0  0
 1  0      0 4271324 275588 2893704    0    0     0     0  896 1408  0  0 100  0  0
 1  0      0 4271324 275588 2893704    0    0     0     0  894 1404  0  0 100  0  0
 1  0      0 4271324 275588 2893704    0    0     0     0  937 1479  0  0 100  0  0
 4  0      0 4271324 275588 2893704    0    0     0     0  960 1542  0  0 100  0  0
 2  0      0 4271324 275588 2893704    0    0     0     0 1173 1520  0  0 99  0  0
 1  0      0 4271324 275588 2893704    0    0     0    12 1021 1417  0  0 100  0  0
 1  0      0 4271324 275588 2893704    0    0     0     0  994 1376  0  0 100  0  0
 1  0      0 4271324 275588 2893704    0    0     0     0 1044 1411  0  0 100  0  0
```
It prints a summary of key server statistics on each line.

vmstat was run with an argument of 1, to print one second summaries. The first line of output (in this version of vmstat) has some columns that show the average since boot, instead of the previous second. 

Look for:
- r: Number of processes running on CPU and waiting for a turn. This provides a better signal than load averages for determining CPU saturation, as it does not include I/O. To interpret: an “r” value greater than the CPU count is saturation.
- free: Free memory in kilobytes. If there are too many digits to count, you have enough free memory. The `free -m` command, better explains the state of free memory.
- si, so: Swap-ins and swap-outs. If these are non-zero, you’re out of memory.
- us, sy, id, wa, st: These are breakdowns of CPU time, on average across all CPUs. They are user time, system time (kernel), idle, wait I/O, and stolen time (by other guests).
- A high system time average, over 20%, can be mean : perhaps the kernel is processing the I/O inefficiently.
- A constant degree of wait I/O points to a disk bottleneck; this is where the CPUs are idle, because tasks are blocked waiting for pending disk I/O. 
- We can treat wait I/O as another form of CPU idle, one that gives a clue as to why they are idle.

4. `mpstat -P ALL <time>`
```
root@15fad6dd8666:/SysPerfLabs/USE_method# mpstat -P ALL 1
Linux 6.6.12-linuxkit (15fad6dd8666) 	02/23/24 	_x86_64_	(11 CPU)

18:55:51     CPU    %usr   %nice    %sys %iowait    %irq   %soft  %steal  %guest  %gnice   %idle
18:55:52     all    0.18    0.00    0.00    0.00    0.00    0.18    0.00    0.00    0.00   99.64
18:55:52       0    0.99    0.00    0.00    0.00    0.00    0.99    0.00    0.00    0.00   98.02
18:55:52       1    0.00    0.00    0.00    0.00    0.00    0.99    0.00    0.00    0.00   99.01
18:55:52       2    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
18:55:52       3    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
18:55:52       4    0.99    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00   99.01
18:55:52       5    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
18:55:52       6    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
18:55:52       7    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
18:55:52       8    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
18:55:52       9    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
18:55:52      10    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
```
This command prints CPU time breakdowns per CPU, which can be used to check for an imbalance. A single hot CPU can be evidence of a single-threaded application.

5. `pidstat <time>`
```
root@15fad6dd8666:/SysPerfLabs/USE_method# pidstat 1
Linux 6.6.12-linuxkit (15fad6dd8666) 	02/23/24 	_x86_64_	(11 CPU)

18:57:50      UID       PID    %usr %system  %guest   %wait    %CPU   CPU  Command
18:57:51        0      1725    1.00    0.00    0.00    0.00    1.00     0  pidstat
```
Shows a rolling stats associated with process IDs, so you can say which PID is using some crazy % utilization

6. `iostat -xz <time`
```
root@15fad6dd8666:/SysPerfLabs/USE_method# iostat -xz 1
Linux 6.6.12-linuxkit (15fad6dd8666) 	02/23/24 	_x86_64_	(11 CPU)

avg-cpu:  %user   %nice %system %iowait  %steal   %idle
           0.25    0.00    0.22    0.02    0.00   99.51

Device            r/s     rkB/s   rrqm/s  %rrqm r_await rareq-sz     w/s     wkB/s   wrqm/s  %wrqm w_await wareq-sz     d/s     dkB/s   drqm/s  %drqm d_await dareq-sz     f/s f_await  aqu-sz  %util
vda              6.07    141.15     0.77  11.23    0.08    23.26   24.00    545.07    52.56  68.65    0.18    22.71    0.38    144.23     0.00   0.00    0.20   378.56   10.44    0.08    0.01   0.37
vdb              0.17     11.69     0.00   0.00    0.25    70.80    0.00      0.00     0.00   0.00    0.00     0.00    0.00      0.00     0.00   0.00    0.00     0.00    0.00    0.00    0.00   0.00
vdc              0.52     36.80     0.00   0.00    0.28    71.19    0.00      0.00     0.00   0.00    0.00     0.00    0.00      0.00     0.00   0.00    0.00     0.00    0.00    0.00    0.00   0.01


avg-cpu:  %user   %nice %system %iowait  %steal   %idle
           0.09    0.00    0.18    0.00    0.00   99.73
```
Help us troubleshoot IO between disks (HDD, SSDs etc)
- r/s, w/s, rkB/s, wkB/s: These are the delivered reads, writes, read Kbytes, and write Kbytes per second to the device. A performance problem may simply be due to an excessive load applied.
- await: The average time for the I/O in milliseconds. This is the time that the application suffers, as it includes both time queued and time being serviced. Larger than expected average times can be an indicator of device saturation, or device problems.
- avgqu-sz: The average number of requests issued to the device. Values greater than 1 can be evidence of saturation (although devices can typically operate on requests in parallel, especially virtual devices which front multiple back-end disks.)
- %util: Device utilization. This is really a busy percent, showing the time each second that the device was doing work. Values greater than 60% typically lead to poor performance (which should be seen in await), although it depends on the device. Values close to 100% usually indicate saturation.

7. `free -m`
```
root@15fad6dd8666:/SysPerfLabs/USE_method# free -m
               total        used        free      shared  buff/cache   available
Mem:            7841         769        4225           1        3046        7071
Swap:           1023           0        1023
```
Look for

- buffers: For the buffer cache, used for block device I/O.
- cached: For the page cache, used by file systems.

We just want to check that these aren’t near-zero in size, which can lead to higher disk I/O (confirm using iostat), and worse performance. 

The “-/+ buffers/cache” provides less confusing values for used and free memory. Linux uses free memory for the caches, but can reclaim it quickly if applications need it. So in a way the cached memory should be included in the free memory column, which this line does.

8. `sar -n DEV <time>`
```
root@15fad6dd8666:/SysPerfLabs/USE_method# sar -n DEV 1
Linux 6.6.12-linuxkit (15fad6dd8666) 	02/23/24 	_x86_64_	(11 CPU)

19:09:02        IFACE   rxpck/s   txpck/s    rxkB/s    txkB/s   rxcmp/s   txcmp/s  rxmcst/s   %ifutil
19:09:03           lo      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
19:09:03        tunl0      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
19:09:03         gre0      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
19:09:03      gretap0      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
19:09:03      erspan0      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
19:09:03      ip_vti0      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
19:09:03     ip6_vti0      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
19:09:03         sit0      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
19:09:03      ip6tnl0      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
19:09:03      ip6gre0      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
19:09:03         eth0      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
```
This tool is used to check network interface throughput: rxkB/s and txkB/s, as a measure of workload, and also to check if any limit has been reached.

9. `sar -n TCP,ETCP <time>`
```
root@15fad6dd8666:/SysPerfLabs/USE_method# sar -n TCP,ETCP 5
Linux 6.6.12-linuxkit (15fad6dd8666) 	02/23/24 	_x86_64_	(11 CPU)

19:11:42     active/s passive/s    iseg/s    oseg/s
19:11:47         0.00      0.00      0.00      0.00

19:11:42     atmptf/s  estres/s retrans/s isegerr/s   orsts/s
19:11:47         0.00      0.00      0.00      0.00      0.00
```

TCP Metrics
- active/s: Number of locally-initiated TCP connections per second (e.g., via connect()).
- passive/s: Number of remotely-initiated TCP connections per second (e.g., via accept()).
- retrans/s: Number of TCP retransmits per second.

The active and passive counts are often useful as a rough measure of server load: number of new accepted connections (passive), and number of downstream connections (active). active == outbound, and passive == inbound, (except when it is a localhost to localhost connection).

Retransmits are a sign of a network or server issue; it may be an unreliable network (e.g., the public Internet), or it may be due a server being overloaded and dropping packets.

10. `top`
```
root@15fad6dd8666:/SysPerfLabs/USE_method# top
top - 19:14:03 up  2:23,  0 user,  load average: 2.68, 2.56, 2.55
Tasks:   2 total,   1 running,   1 sleeping,   0 stopped,   0 zombie
%Cpu(s):  0.2 us,  0.1 sy,  0.0 ni, 99.6 id,  0.0 wa,  0.0 hi,  0.1 si,  0.0 st
MiB Mem :   7841.1 total,   4225.3 free,    769.1 used,   3046.7 buff/cache
MiB Swap:   1024.0 total,   1024.0 free,      0.0 used.   7072.0 avail Mem

  PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND
 1798 root      20   0  293868   8452   3456 R   1.0   0.1   0:00.08 top
    1 root      20   0  289572   6020   2688 S   0.0   0.1   0:00.59 bash
```

Just a overall summary view. Very helpful, but might be hard to track patterns over time.