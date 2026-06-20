# 04 — Multilevel Feedback Queue (MLFQ) Scheduler

## Overview

Implemented a **Multilevel Feedback Queue (MLFQ) CPU scheduling algorithm** that dynamically adjusts process priority based on CPU usage patterns. The scheduler uses two queues with different scheduling policies and automatically demotes CPU-bound processes to prevent starvation of interactive/IO-bound jobs.

## Problem

A single scheduling algorithm can't optimize for all workload types:
- **Interactive jobs** need low latency (fast response times)
- **CPU-bound jobs** need high throughput (efficient CPU utilization)
- **A mix of both** requires the scheduler to *learn* which is which — without prior knowledge

MLFQ solves this by observing process behavior: processes that use their full time quantum are likely CPU-bound and get demoted to a lower-priority queue.

## Architecture

```
                    ┌──────────────────────────────────────┐
                    │         MLFQ Scheduler               │
                    │                                      │
   New Process ──►  │  Queue A (High Priority)             │
                    │  Policy: FCFS (First Come First Serve)│
                    │  Time Quantum: 5                     │
                    │                                      │
                    │         │ demotion after N turns      │
                    │         ▼                             │
                    │  Queue B (Low Priority)               │
                    │  Policy: SJF (Shortest Job First)    │
                    │  Time Quantum: 40                    │
                    │                                      │
                    │  Dispatch Ratio: A dispatches before  │
                    │  B up to the ratio limit              │
                    └──────────────────────────────────────┘
                                    │
                                    ▼
                               ┌────────┐
                               │  CPU   │
                               └────────┘
```

## Design Decisions

### Two-Queue Strategy
- **Queue A** uses FCFS with a short quantum (5 units) — optimized for interactive processes that do short CPU bursts
- **Queue B** uses SJF with a long quantum (40 units) — optimized for CPU-bound processes that need extended computation

### Demotion Criteria
A process in Queue A that exhausts its time quantum `N` times (configurable) is demoted to Queue B. This is the key heuristic: if a process keeps using its full quantum, it's likely CPU-bound and should be in the throughput-optimized queue.

Setting `N = 0` disables demotion (all processes stay in Queue A).

### Dispatch Ratio
Controls fairness between queues. A ratio of `R` means Queue A gets dispatched `R` times before Queue B gets one turn. Setting `R = 0` gives Queue A absolute priority (Queue B only runs when A is empty).

### SJF in Queue B
Queue B sorts by remaining execution time using `stable_sort`, ensuring:
1. Shortest remaining job runs first (minimizes average wait time)
2. Ties are broken by entry time into Queue B (FIFO among equal-length jobs)

## Implementation Highlights

### Main Simulation Loop

Each clock tick:
1. **Check completion** — if the running process finished, record it
2. **Check preemption** — if the running process used its full quantum, preempt it
3. **Dispatch** — pick the next process from Queue A or B based on dispatch ratio
4. **Queue returning processes** — put preempted processes back (with demotion check)
5. **Fetch new jobs** — read the next job from input (if available at this tick)
6. **Update counters** — decrement remaining time, increment wait times for queued processes
7. **Termination check** — exit when no more input, both queues empty, and CPU idle

### Key Metrics Tracked

| Metric | Description |
|---|---|
| End Time | Total simulation time (execution + idle) |
| Processes Completed | Number of finished processes |
| Total Execution Time | CPU busy time |
| Idle Time | CPU idle time |
| Longest Wait Time | Worst-case wait (starvation indicator) |
| Average Wait Time | Mean wait across all processes |

## Usage

```bash
g++ -o scheduler main.cpp
./scheduler <input_file> <demotion_criteria> <dispatch_ratio>
```

**Parameters:**
- `input_file` — one job per line: burst time (integer) or `idle` for no arrival
- `demotion_criteria` — number of Queue A turns before demotion to B (0 = no demotion)
- `dispatch_ratio` — how many Queue A dispatches before one Queue B dispatch (0 = A has absolute priority)

**Example:**
```bash
./scheduler TestCase2.txt 3 2
```

### Input Format

Each line represents one clock tick. A number means a new process arrives with that burst time. `idle` means no new process arrives at that tick.

```
31          # tick 0: process arrives with burst time 31
31          # tick 1: process arrives with burst time 31
idle        # tick 2: no new process
29          # tick 3: process arrives with burst time 29
```

### Sample Output

```
End Time:                96
Processes Completed:     3
Total execution time:    91
Idle time:               5
Longest Wait Time:       35
Average Wait Time:       17
Total Wait Time:         51
```

## Test Cases

| File | Description |
|---|---|
| `TestCase1.txt` | Single process with burst time 70 |
| `TestCase2.txt` | Three processes arriving at different ticks |
| `TestCase3.txt` | Two processes with long idle gap between arrivals |

## Real-World Parallels

- **Linux CFS** (Completely Fair Scheduler) uses a similar concept of dynamically adjusting priority based on CPU consumption via "virtual runtime"
- **Database query schedulers** prioritize short queries (OLTP) over long-running analytics (OLAP) — the same interactive-vs-batch tradeoff
- **IO schedulers** (Linux `mq-deadline`, `bfq`) use multilevel queuing to balance latency and throughput

## Files

| File | Description |
|---|---|
| `main.cpp` | Complete MLFQ scheduler implementation |
| `TestCase1.txt` | Test: single long-running process |
| `TestCase2.txt` | Test: multiple concurrent processes |
| `TestCase3.txt` | Test: processes with idle gaps |
