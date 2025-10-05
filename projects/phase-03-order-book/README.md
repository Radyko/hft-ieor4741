# Order Book and Order Manager
### Overview
The program tracks bids and asks in a limit order book, updates price levels from a feed, and simulates placing and filling buy orders based on market conditions.

It is organized into two main components:

* MarketSnapshot – Maintains the best bid and ask quotes.
* OrderManager – Tracks all placed orders, their status, and fill progress.

The main driver (main.cpp) reads market updates from a feed file, updates the snapshot, and decides when to place new orders.

### Logic Summary
Each feed update modifies the MarketSnapshot.
If the spread (ask - bid) becomes less than 0.05,
→ a new BUY order is placed at the best bid.
Executions reduce the open quantity and update order status:
* New
* PartiallyFilled
* Filled
  
Filled or cancelled orders are removed from the active list. At the end, all active orders are printed.

### Build and Run
From the phase-03-order-book directory:
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp market_snapshot.cpp order_manager.cpp -o driver
./driver

### Description of Files
market_snapshot.h / .cpp	Maintains the live order book.
order_manager.h / .cpp	Tracks and updates orders.
main.cpp	Driver and trading logic.
sample_feed.txt	Example market data feed.
