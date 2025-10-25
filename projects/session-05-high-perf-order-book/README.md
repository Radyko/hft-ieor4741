# High-Performance Order Book

This project implements a simplified **limit order book** designed for speed, memory efficiency, and correctness.  
It models how real electronic trading systems store and match buy/sell orders.

---

## Overview

The `OrderBook` maintains two sides:
- **Bids** (buy orders)
- **Asks** (sell orders)

Each side stores multiple **price levels**, and each price level contains all orders at that price.

### Data Structures
- **`Order`** – Holds `id`, `price`, `quantity`, and `side`.
- **`PriceLevel`** – Aggregates total quantity and orders for a given price.
- **`OrderBook`** – Manages all price levels, order insertion, amendment, and deletion.

Optimizations:
- `alignas(64)` cache-line alignment for core structs
- Optional **Struct-of-Arrays (SoA)** layout for improved cache locality
- No mid-vector erasures; uses free-list recycling for stable indices

---

## Build & Run

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/session_05_high_perf_order_book
```

---

## Benchmark Example

```text
==== OrderBook Benchmark ====
Events:   1000000
Batches:  1000 (size 1000)
Total:    47 s
Average:  47970.5 ns / event
Min:      250 ns
Median:   750 ns
Max:      30407583 ns

Histogram (per-event latency):
  <= 500 ns : 24083
  <= 1000 ns : 739522
  <= 2000 ns : 91906
  <= 4000 ns : 37242
  <= 8000 ns : 7868
  <= 16000 ns : 3233
  <= 32000 ns : 3964
  <= 64000 ns : 6479
  <= 128000 ns : 10360
  <= 256000 ns : 15976
  <= 512000 ns : 22148
  <= 1024000 ns : 25049
  <= 2048000 ns : 12138
  <= 4096000 ns : 24
  <= 8192000 ns : 5
  <= 16384000 ns : 2
  <= 32768000 ns : 1
  <= 65536000 ns : 0
  <= 131072000 ns : 0
  <= 262144000 ns : 0
  <= 524288000 ns : 0
  >  524288000 ns : 0
```


*(Numbers will vary depending on CPU and compiler optimization. This result is on a Macbook Pro M1 14.")*