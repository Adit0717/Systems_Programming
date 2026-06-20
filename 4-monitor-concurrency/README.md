# 03 — Monitor-Based Concurrency: Producer-Consumer with Condition Variables

## Overview

Implemented a **monitor-based solution** to the classic Producer-Consumer problem using condition variables with Mesa-style semantics. This involved building both the synchronization primitives (`Condition.Wait()`, `Signal()`, `Broadcast()`) in `Synch.k` and the `BufferMonitor` class that coordinates multiple producer and consumer threads sharing a bounded buffer.

## Problem

Multiple producer threads generate data items and place them into a shared fixed-size buffer. Multiple consumer threads remove items from the buffer. The system must guarantee:

1. **Mutual exclusion** — only one thread modifies the buffer at a time
2. **No buffer overflow** — producers block when the buffer is full
3. **No buffer underflow** — consumers block when the buffer is empty
4. **Liveness** — blocked threads are woken up when conditions change

## Architecture

```
                    ┌──────────────────────────────────────────┐
                    │            BufferMonitor                 │
                    │                                          │
  Producer-A ──►    │  ┌─────────────────────────────────┐     │    ──► Consumer-1
  Producer-B ──►    │  │  Bounded Buffer (size 7)        │     │    ──► Consumer-2
  Producer-C ──►    │  │  [A][B][C][D][ ][ ][ ]          │     │    ──► Consumer-3
  Producer-D ──►    │  │   ▲              ▲              │     │
  Producer-E ──►    │  │   bufferNextOut  bufferNextIn    │     │
                    │  └─────────────────────────────────┘     │
                    │                                          │
                    │  monitorMutex  ── ensures mutual exclusion│
                    │  not_full      ── producers wait here     │
                    │  not_empty     ── consumers wait here     │
                    └──────────────────────────────────────────┘
```

## Implementation Details

### Condition Variables (Synch.k)

The `Condition` class implements Mesa-style semantics:

**`Wait(mutex)`:**
1. Verify the calling thread holds the mutex (safety check)
2. Release the mutex (so other threads can enter the monitor)
3. Add self to the condition's waiting list
4. Sleep (yield the CPU)
5. *Upon wakeup:* the caller must re-acquire the mutex

**`Signal(mutex)`:**
1. Verify the calling thread holds the mutex
2. Remove the oldest waiting thread (if any) from the waiting list
3. Set its status to READY and add it to the scheduler's ready list
4. Release the mutex

**`Broadcast(mutex)`:** Same as Signal, but wakes *all* waiting threads.

> **Mesa vs. Hoare semantics:** In Mesa-style monitors (used here), a signaled thread doesn't run immediately — it's just placed on the ready list. By the time it actually runs, the condition may no longer be true. That's why all waits use `while` loops, not `if` statements.

### BufferMonitor (Main.k)

**`addItem(c)`** — Producer entry point:
```
Lock the monitor mutex
while buffer is full:
    Wait on not_full condition      -- releases lock, sleeps
    Re-acquire lock after wakeup
Place character at bufferNextIn position
Advance bufferNextIn (circular: modulo BUFFER_SIZE)
Increment bufferLength
Signal not_empty condition          -- wake a waiting consumer
```

**`removeItem()`** — Consumer entry point:
```
Lock the monitor mutex
while buffer is empty:
    Wait on not_empty condition     -- releases lock, sleeps
    Re-acquire lock after wakeup
Read character from bufferNextOut position
Advance bufferNextOut (circular: modulo BUFFER_SIZE)
Decrement bufferLength
Signal not_full condition           -- wake a waiting producer
```

### Circular Buffer

The buffer uses modular arithmetic for wrap-around:
```
bufferNextIn  = (bufferNextIn + 1) % BUFFER_SIZE
bufferNextOut = (bufferNextOut + 1) % BUFFER_SIZE
```

This is the same ring buffer pattern used in production systems like Linux's `kfifo`, network packet buffers, and database WAL (Write-Ahead Log) buffers.

## Key Concepts Demonstrated

- **Monitor pattern** — encapsulating shared state with synchronized entry methods
- **Condition variables** — `Wait`, `Signal`, `Broadcast` with Mesa semantics
- **Bounded buffer** — fixed-size circular buffer with producer/consumer coordination
- **Deadlock prevention** — careful ordering of lock acquire/release to avoid circular waits
- **The `while` loop pattern** — always re-check conditions after wakeup (Mesa semantics requirement)

## Real-World Parallels

| This Project | Production Equivalent |
|---|---|
| Bounded buffer with wait/signal | Database write-ahead log buffers, Kafka partition buffers |
| Producer-Consumer pattern | Message queue systems (RabbitMQ, SQS), database replication streams |
| Condition variables | `pthread_cond_wait` / `pthread_cond_signal` in POSIX |
| Monitor mutex | Java's `synchronized` blocks, Go's `sync.Cond` |

## Files

| File | Description |
|---|---|
| `Synch.k` | **Core implementation** — Mutex + Condition variables (`Wait`, `Signal`, `Broadcast`) |
| `Synch.h` | Synchronization class declarations |
| `Main.k` | **BufferMonitor implementation** — `Init()`, `addItem()`, `removeItem()` + test harness |
| `Main.h` | Main declarations |
| `Thread.k` / `Thread.h` | Thread management |
| `List.k` / `List.h` | Queue data structure for waiting threads |
| `System.k` / `System.h` | System utilities |
| `Runtime.s` | Assembly runtime |
| `Switch.s` | Context switching |
| `makefile` | Build configuration |

## Building & Testing

```bash
make
# Load into BLITZ emulator
# Output shows interleaved producer/consumer activity with buffer state visualization
# 5 producers each add 7 characters, 3 consumers remove them
# Buffer contents are printed after each add/remove operation
```

**Note:** The Dining Philosophers problem is defined in the codebase but is not implemented in this version. The focus of this project was the Producer-Consumer monitor solution.
