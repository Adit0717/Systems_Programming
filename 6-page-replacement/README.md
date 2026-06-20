# 05 — Page Replacement Algorithms with Inverted Page Table

## Overview

Implemented and benchmarked three page replacement algorithms — **Second Chance**, **LRU (Least Recently Used)**, and **Enhanced Second Chance** — using an inverted page table data structure. The simulator processes real memory access traces (~11MB, hundreds of thousands of memory references) and compares page fault rates across different memory sizes and page sizes.

## Problem

When physical memory is full and a new page needs to be loaded, the OS must decide which existing page to **evict**. The choice of eviction algorithm directly impacts performance:
- **Too many page faults** → constant disk I/O → system thrashes
- **Optimal eviction** → keeps the "right" pages in memory → minimal disk access

This is the exact same problem faced by:
- **Database buffer pools** — which data pages to keep in memory vs. flush to disk
- **Web caches** — which responses to keep cached
- **CDN edge caches** — which content to serve locally vs. fetch from origin

## Algorithms Implemented

### 1. Second Chance (Clock Algorithm)

A FIFO-based algorithm enhanced with a "second chance" bit:

```
Pointer cycles through frames in circular order:
  If frame's ref_bit == 0 → EVICT this frame (victim found)
  If frame's ref_bit == 1 → Set ref_bit = 0, advance pointer (give second chance)
```

**Trade-off:** Simple and fast, but doesn't consider recency of access.

### 2. LRU — Least Recently Used (Timestamp Approach)

Evicts the page that hasn't been used for the longest time:

```
Each frame stores a timestamp (incremented on every memory access)
On page fault with no free frame:
  Scan all frames → find the one with the SMALLEST timestamp
  Evict that frame (it was accessed least recently)
```

**Trade-off:** Optimal approximation, but requires scanning all frames on every eviction.

### 3. Enhanced Second Chance

Extends the Clock algorithm by considering both reference bit AND dirty bit, creating a 4-class priority system:

```
Priority for eviction (lowest = evict first):
  (0, 0) → not recently used, not modified     ← best victim
  (0, 1) → not recently used, but modified     ← need write-back
  (1, 0) → recently used, not modified         ← might be needed
  (1, 1) → recently used AND modified          ← worst victim

Two-pass approach:
  Pass 1: Look for (0,0) frames — clean and unused
  Pass 2: Look for (0,1) frames — dirty but unused, clear ref bits along the way
  Repeat if needed
```

**Trade-off:** Minimizes expensive write-back operations by preferring clean pages.

## Data Structure: Inverted Page Table

Instead of a traditional per-process page table, this uses an **inverted page table** — one entry per physical frame:

```c++
struct Frame {
    int page_number;   // -1 if frame is empty
    int ref_bit;       // referenced since last check? (Second Chance, Enhanced)
    int timeStamp;     // last access time (LRU)
    int dirty_bit;     // modified since load? (Enhanced Second Chance)
};
```

The inverted table is indexed by frame number, with linear search for page lookup. This mirrors how hardware-assisted inverted page tables work (e.g., PowerPC, IA-64).

## Usage

```bash
g++ -o pager main.cpp
./pager <trace_file> <algorithm> <memory_size_KB> <page_size_KB>
```

**Parameters:**
- `trace_file` — memory access trace (hex address + R/W per line)
- `algorithm` — `S` (Second Chance), `L` (LRU), `E` (Enhanced Second Chance)
- `memory_size_KB` — total physical memory in KB (e.g., 64, 128, 256)
- `page_size_KB` — page size in KB (e.g., 4, 16, 32, 64)

**Example:**
```bash
./pager trace1.txt L 128 16
# Output: Memory size: 128, Page size: 16, Page Faults: 2347
```

### Trace File Format

Each line contains an 8-character hex address and a R/W operation:
```
190a7c20 R
08a02b40 W
3ff01a00 R
```

### Batch Testing

The included `script.sh` runs all 72 combinations (3 algorithms × 2 traces × 3 memory sizes × 4 page sizes):

```bash
chmod +x script.sh
./script.sh
```

## Analysis

The simulator enables comparison across multiple dimensions:

| Variable | Effect on Page Faults |
|---|---|
| **↑ Memory size** | ↓ Page faults (more frames available) |
| **↑ Page size** | ↓ Page faults (fewer pages needed) but ↑ internal fragmentation |
| **LRU vs Second Chance** | LRU generally fewer faults (better approximation of optimal) |
| **Enhanced vs Basic Second Chance** | Similar fault rates, but Enhanced minimizes write-backs |

## Key Concepts Demonstrated

- **Page replacement policies** — the core memory management decision in any virtual memory system
- **LRU as a cache eviction strategy** — the same algorithm used in database buffer pools (InnoDB, RocksDB) and caching layers (Redis, Memcached)
- **Reference bit / dirty bit tracking** — hardware-assisted hints for OS memory management
- **Clock algorithm** — the practical approximation of LRU used in most real operating systems
- **Inverted page table** — space-efficient page table design used in real hardware
- **Benchmarking methodology** — systematic comparison across parameter combinations

## Real-World Parallels

| This Project | Production Equivalent |
|---|---|
| LRU page replacement | Redis `allkeys-lru` eviction, InnoDB buffer pool LRU list |
| Second Chance / Clock | Linux kernel page reclaim (`kswapd`), PostgreSQL buffer ring |
| Enhanced Second Chance (dirty bit) | Database "clean page" preference during buffer eviction |
| Inverted page table | Hash-based page tables in modern OS kernels |
| Page fault counting | Cache hit/miss ratio monitoring in production systems |

## Files

| File | Description |
|---|---|
| `main.cpp` | Complete implementation — all 3 algorithms + address parsing + main driver |
| `script.sh` | Batch runner — executes all 72 parameter combinations |
