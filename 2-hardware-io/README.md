# 01 — Hardware I/O: Implementing `GetCh()` in Assembly

## Overview

Implemented a character input routine (`GetCh()`) in BLITZ assembly language, enabling the OS kernel to read characters from the serial port by directly interfacing with hardware device registers.

## Problem

The BLITZ runtime (`Runtime.s`) provides output functions (`print`, `printChar`, etc.) but lacks a character *input* function. Without `GetCh()`, user programs cannot read keyboard input from the serial device. The goal was to implement this missing primitive at the assembly level, understanding the full path from hardware register → CPU register → stack frame → KPL caller.

## Implementation Details

### How Serial I/O Works in BLITZ

The BLITZ architecture exposes serial I/O through two memory-mapped registers:

| Register | Address | Purpose |
|---|---|---|
| `SERIAL_STAT` | `0x00FFFF00` | Status register — bit 0 indicates data is available |
| `SERIAL_DATA` | `0x00FFFF04` | Data register — contains the received character |

### `GetCh()` Algorithm

```
1. Save registers (r2–r5) to preserve caller context
2. Load SERIAL_STAT and SERIAL_DATA addresses into registers  
3. Poll: Load status register value, test bit 0
   - If 0 → no data yet, loop back (busy-wait)
   - If 1 → data is available, proceed
4. Load character from SERIAL_DATA
5. Store character to the return-value slot on the stack [r15 + 20]
6. Restore registers and return
```

### Stack Frame Layout

```
┌─────────────────────┐  ← r15 (stack pointer)
│  saved r5           │  r15 + 0
│  saved r4           │  r15 + 4
│  saved r3           │  r15 + 8
│  saved r2           │  r15 + 12
│  return address     │  r15 + 16
│  return value slot  │  r15 + 20  ← character stored here
└─────────────────────┘
```

The key insight is that `storeb r2, [r15 + 20]` places the character directly into the return-value position expected by the KPL calling convention, so the caller receives the character as the function's return value.

## Key Concepts Demonstrated

- **Polling I/O** — busy-waiting on a hardware status register (vs. interrupt-driven I/O)
- **Memory-mapped I/O** — accessing device registers through fixed memory addresses
- **Register preservation** — saving/restoring caller-owned registers across function calls
- **Calling conventions** — understanding stack layout to return values correctly
- **Hardware-software interface** — bridging the gap between physical devices and high-level language constructs

## Files

| File | Description |
|---|---|
| `Runtime.s` | Runtime support — contains the `GetCh()` implementation (line ~499) |
| `echoprime.k` | Test program — reads characters and echoes them back, quit with 'q' |
| `HelloWorld.k` | Basic test program |
| `System.h` / `System.k` | KPL system utilities |
| `makefile` | Build configuration |

## Building

Requires the BLITZ toolchain. See [BLITZ project page](http://web.cecs.pdx.edu/~harry/Blitz/).

```bash
make
# Load into BLITZ emulator and run
```
