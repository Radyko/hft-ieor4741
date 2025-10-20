# HFT Trading System

This project simulates a simple high-frequency trading (HFT) system.  
It generates mock market data, creates and matches buy/sell orders, logs trades, and measures tick-to-trade latency.

---

## Overview

The system has the following main components:

- **MarketData** – generates random bid and ask prices for a stock (AAPL).
- **OrderManager** – assigns unique order IDs and manages order creation.
- **OrderBook** – stores buy and sell orders by price.
- **MatchingEngine** – checks for matches between buy and sell orders.
- **TradeLogger** – logs executed trades to a CSV file.
- **Latency Analysis** – measures and reports timing for each tick-to-trade cycle.

---

## Build and Run

```bash
mkdir build && cd build
cmake ..
make
./phase_04_hft_trading_system
```

This creates a file called `trades.csv` containing the simulated trades.

---

## Example Output

```
Tick-to-Trade Latency (nanoseconds):
Min: 2084
Max: 350291
Mean: 91433.8
StdDev: 53516
P99: 201167
Best Bid: 225 (60)
Best Ask: 218 (56)

Trades have been logged to trades.csv
```

---

## Notes

- Uses `std::mt19937` for random price and quantity generation.
- All timing uses `std::chrono::high_resolution_clock`.
- The simulation is single-threaded and meant for benchmarking logic, not real trading.
- To test scalability, you can change the number of ticks in `main.cpp`.
