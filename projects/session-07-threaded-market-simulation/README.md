# Threaded Market Simulation

This project simulates a simple multi-threaded trading system in C++.

It has three main threads:

1. **Market Data Feed** – generates random price updates.
2. **Strategy Engine** – checks for large price changes and creates BUY/SELL orders.
3. **Order Router** – logs the generated orders to a CSV file.

The threads communicate through queues using mutexes and condition variables.

---

## How to Build

```bash
mkdir build && cd build
cmake ..
make
