# 02 — Mutex Implementation: Building Synchronization from Scratch

## Overview

Implemented a complete Mutex (mutual exclusion lock) from scratch in KPL for the BLITZ operating system kernel, including `Init()`, `Lock()`, `Unlock()`, and `IsHeldByCurrentThread()`. This provides the fundamental building block for all higher-level synchronization — condition variables, monitors, and semaphores all depend on correct mutex behavior.

## Problem

Multiple kernel threads running concurrently need exclusive access to shared resources (data structures, I/O devices, etc.). Without a correct mutex, race conditions lead to data corruption. The challenge is implementing a lock that is:
1. **Correct** — no two threads can hold the lock simultaneously
2. **Safe** — no deadlocks from a thread re-locking its own mutex
3. **Fair** — waiting threads are served in FIFO order
4. **Atomic** — the lock/unlock operations themselves cannot be interrupted mid-execution

## Implementation Details

### Achieving Atomicity

Since BLITZ doesn't provide atomic hardware instructions (like `compare-and-swap`), atomicity is achieved by **disabling interrupts** during critical sections of the lock/unlock code:

```
oldIntStat = SetInterruptsTo(DISABLED)   -- begin atomic section
... critical mutex operations ...
oldIntStat = SetInterruptsTo(oldIntStat) -- restore previous interrupt state
```

This pattern preserves the previous interrupt state, making the code safe to call regardless of whether interrupts were previously enabled or disabled.

### Lock()

```
Lock():
  disable interrupts
  if this thread already holds the lock → FatalError (prevent self-deadlock)
  while lock is held by another thread:
      add self to waitingThreads list
      Sleep()                            -- yields CPU, will be woken by Unlock()
  set heldBy = currentThread
  restore interrupts
```

#### The Race Condition Bug

My initial implementation used an `if` statement instead of a `while` loop:

```
-- BUGGY VERSION (race condition):
if heldBy == null
    heldBy = currentThread
else
    waitingThreads.AddToEnd(currentThread)
    currentThread.Sleep()
    heldBy = currentThread              -- ← Problem: assumes lock is free after wakeup!
endIf
```

**Why this breaks:** When a sleeping thread is woken up by `Unlock()`, it resumes execution right after `Sleep()`. But between being woken and actually running, *another thread might have already acquired the lock*. The `if` version blindly sets `heldBy = currentThread` without re-checking, violating mutual exclusion.

**The fix:** Use a `while` loop that re-checks the condition after every wakeup — the classic pattern for handling **spurious wakeups** in concurrent programming:

```
while heldBy != null
    waitingThreads.AddToEnd(currentThread)
    currentThread.Sleep()
endWhile
heldBy = currentThread
```

### Unlock()

```
Unlock():
  disable interrupts
  if current thread doesn't hold the lock → FatalError
  nextThread = waitingThreads.Remove()
  if nextThread exists:
      set nextThread.status = READY
      add nextThread to readyList        -- wakes it up for scheduling
  set heldBy = null                      -- release the lock
  restore interrupts
```

### IsHeldByCurrentThread()

Simple ownership check: `return heldBy == currentThread`. Used as a safety guard in `Lock()` and `Unlock()` to catch programming errors early.

## Key Concepts Demonstrated

- **Mutual exclusion** — ensuring only one thread accesses a shared resource at a time
- **Interrupt-based atomicity** — using interrupt disable/enable as a synchronization mechanism in kernel code
- **Race condition identification and resolution** — the `if` → `while` fix is a textbook concurrency bug
- **Spurious wakeup handling** — always re-check conditions in a loop after `Sleep()`/`Wait()`
- **FIFO fairness** — waiting threads are queued and served in order

## Real-World Parallels

This mutex implementation mirrors the same patterns used in:
- **`pthread_mutex_lock`** in POSIX threads
- **Database row-level locks** — PostgreSQL's `LWLock` uses similar spin-then-sleep patterns
- **Go's `sync.Mutex`** — also uses a combination of spinning and sleeping with FIFO fairness
- **Java's `ReentrantLock`** — ownership tracking via `IsHeldByCurrentThread()` is directly analogous

## Files

| File | Description |
|---|---|
| `Synch.k` | **Main implementation** — Mutex (`Init`, `Lock`, `Unlock`, `IsHeldByCurrentThread`) + Semaphore + Condition variables |
| `Synch.h` | Header file with class/field declarations |
| `Main.k` | Test harness — 7 concurrent threads incrementing a shared integer (TestMutex) |
| `Thread.k` / `Thread.h` | Thread implementation (context switching, scheduling) |
| `List.k` / `List.h` | Linked list used for waiting thread queues |
| `System.k` / `System.h` | System utilities |
| `Runtime.s` | Assembly runtime support |
| `Switch.s` | Context switch implementation |
| `makefile` | Build configuration |

## Building & Testing

```bash
make
# Load into BLITZ emulator
# Expected output: 70 consecutively numbered lines (7 threads × 10 iterations)
# If mutex is incorrect, numbers will be duplicated or out of order
```
