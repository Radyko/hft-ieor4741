Phase 03 – Order Book and Order Manager
This project builds on earlier assignments to simulate a simple order book and order management system for high-frequency trading (HFT).
Overview
The program keeps track of bids and asks in a limit order book, updates price levels from a feed, and simulates placing and filling buy orders based on market conditions.
It is divided into two main components:
MarketSnapshot – Maintains the best bid and ask quotes.
OrderManager – Tracks all placed orders, their status, and fill progress.
The main program (main.cpp) reads a simple market feed from a text file, updates the snapshot, and decides when to place new orders.
Feed Format
The input file (sample_feed.txt) contains tokenized market updates, one per line:
BID <price> <qty>
ASK <price> <qty>
EXECUTION <id> <filled_qty>
Example:
BID 100.10 300
ASK 100.20 250
BID 100.10 0
ASK 100.25 200
EXECUTION 1 10
EXECUTION 1 40
A BID or ASK updates or removes a price level (quantity 0 removes the level).
EXECUTION represents a fill for an existing order ID.
Logic Summary
Each new feed update modifies the MarketSnapshot.
If the spread (ask − bid) becomes less than 0.05, a new BUY order is placed at the best bid.
Executions reduce the open quantity and update order status (New, PartiallyFilled, or Filled).
Filled or cancelled orders are removed from the active list.
At the end, all active orders are printed.
How to Build and Run
Compile everything from the phase-03-order-book directory:
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp market_snapshot.cpp order_manager.cpp -o driver
./driver
Make sure sample_feed.txt is in the same directory as the executable.
Example Output
Placed BUY order at 100.10 (id=1)
Order 1 fully filled (10).
Placed BUY order at 100.10 (id=2)

Final active orders:
Active orders:
ID 2 | Side: Buy | Price: 100.1 | Qty: 10 | Filled: 0 | Status: New
Files
market_snapshot.h / .cpp – Maintains the live order book.
order_manager.h / .cpp – Tracks and updates orders.
main.cpp – Driver and trading logic.
sample_feed.txt – Example market data feed.
Notes
This project focuses on memory-safe handling using std::unique_ptr and clean order lifecycle management.
It demonstrates basic trading logic and order tracking, not actual exchange connectivity.