# Operating Systems — From Primitives to Algorithms

A collection of systems programming projects implemented during my Operating Systems coursework, covering the full stack from hardware I/O and concurrency primitives to CPU scheduling algorithms and virtual memory management.

These projects demonstrate hands-on experience with the low-level systems concepts that underpin modern infrastructure — concurrency control (mutexes, monitors, condition variables), resource scheduling (multilevel feedback queues), and cache/buffer management (LRU, Second Chance page replacement).

## Projects

| # | Project | Language | Key Concepts |
|---|---------|----------|--------------|
| 1 | [Hardware I/O — `GetCh()`](01-hardware-io/) | BLITZ Assembly | Polling I/O, serial device registers, stack frame management |
| 2 | [Mutex Implementation](02-mutex-implementation/) | KPL (Kernel Programming Language) | Mutual exclusion, interrupt-based atomicity, race condition debugging |
| 3 | [Monitor-Based Concurrency](03-monitor-concurrency/) | KPL | Condition variables (Mesa semantics), Producer-Consumer problem, bounded buffers |
| 4 | [Multilevel Feedback Queue Scheduler](04-mlfq-scheduler/) | C++ | MLFQ scheduling, FCFS vs SJF, time quantum, demotion policies, dispatch ratios |
| 5 | [Page Replacement Algorithms](05-page-replacement/) | C++ | Second Chance, LRU (timestamp), Enhanced Second Chance, inverted page tables |

## Technology Context

**Projects 1–3** are built on the [BLITZ system](http://web.cecs.pdx.edu/~harry/Blitz/) — an educational operating system project by Harry Porter at Portland State University. BLITZ includes a custom CPU architecture, assembler, and the KPL (Kernel Programming Language). These projects involve modifying the OS kernel itself — implementing synchronization primitives from scratch at the interrupt handler level.

**Projects 4–5** are standalone C++ implementations of core OS algorithms, designed to be compiled and run on any standard system with a C++ compiler.

## How These Map to Real-World Systems

These aren't just academic exercises — the concepts directly underpin production infrastructure:

| OS Concept (This Repo) | Production System Equivalent |
|---|---|
| **Mutex / Condition Variables** | Database locking (PostgreSQL row locks, ScyllaDB lightweight transactions) |
| **Producer-Consumer with Bounded Buffer** | Message queues, database write-ahead log buffers |
| **MLFQ Scheduling** | Linux CFS scheduler, database query/IO prioritization |
| **LRU Page Replacement** | Database buffer pool eviction (InnoDB buffer pool, RocksDB block cache) |
| **Second Chance / Clock Algorithm** | OS page cache management, web server cache eviction |
| **Inverted Page Table** | Hash-based index structures in databases |

## Building & Running

### BLITZ Projects (1–3)
These require the BLITZ toolchain (assembler + emulator). See the [BLITZ project page](http://web.cecs.pdx.edu/~harry/Blitz/) for setup instructions.

```bash
cd 01-hardware-io/
make
# Run in BLITZ emulator
```

### C++ Projects (4–5)
Requires a C++ compiler with C++11 support.

```bash
# MLFQ Scheduler
cd 04-mlfq-scheduler/
g++ -o scheduler main.cpp
./scheduler TestCase1.txt 3 2    # <input_file> <demotion_criteria> <dispatch_ratio>

# Page Replacement
cd 05-page-replacement/
g++ -o pager main.cpp
./pager trace.txt S 128 16       # <trace_file> <algorithm: S|L|E> <memory_KB> <page_size_KB>
```

## License

[MIT](LICENSE)
