# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

plancktimelib is a C library for representing and manipulating time in Planck time units using arbitrary-precision arithmetic (GMP). It provides conversions between Planck time (`ptm`/`ptime_t`) and POSIX `timespec`, arithmetic operations, and string formatting.

## Build Commands

```bash
# Configure and build
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build

# Run test executable
./build/test/testpc

# Install (system-wide)
sudo cmake --install build
```

## Dependencies

- **GMP** (GNU Multiple Precision Arithmetic Library) - required, found via custom `CMake/FindGMP.cmake`
- CMake >= 3.16
- C90 standard

## Architecture

### Core Types (`include/planckclock.h`)

- **`ptime_t`** (`uint64_t`) - Low-precision timestamp representing time in "novs" (10^24 Planck times)
- **`ptm`** - High-precision timestamp struct with 13 `uint16_t` fields, each representing 4 hex digits at increasing powers of 10^4, covering the full range from units (10^0) up to "doe" (10^48 Planck times). Stored little-endian (lowest significance first).
- **`PTIME_PWR`** enum - Hex power constants for indexing into `ptm` fields

### Key Constants (`src/planckclock.c`)

- `TIME_PER_NSECOND` - Planck times per nanosecond (~1.855e34)
- `TIME_SINCE_BIG_BANG` - Planck times from Big Bang to Unix epoch

### API Categories

1. **Conversions** - Between `ptm`/`ptime_t` (self), and to/from `struct timespec`
2. **Arithmetic** - Add/subtract operations between all type combinations (`ptm`, `ptime_t`, `timespec`)
3. **Formatting** - `strfptm`/`strfptime` format Planck times to strings using `%A`-`%Z` specifiers (each letter maps to a byte position in the `ptm` struct, `%Z` = least significant)
4. **Other** - `ptime_now()` gets current time via `clock_gettime`

### Convention

- Functions returning `int` use 1 for success, 0 for failure (overflow/underflow/out-of-range)
- Naming pattern: `<output_type>_<operation>_<input_type>` (e.g., `ts_at_ptm` converts ptm to timespec)
