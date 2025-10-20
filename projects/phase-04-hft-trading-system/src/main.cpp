#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>

#include "../include/MarketData.hpp"
#include "../include/OrderManager.hpp"
#include "../include/OrderBook.hpp"
#include "../include/MatchingEngine.hpp"
#include "../include/TradeLogger.hpp"
#include "../include/Timer.hpp"
#include "../include/TestLatency.hpp"

using PriceType = double;
using OrderIdType = int;

int main() {
    const int num_ticks = 10000;

    GenerateMarketData feed;
    OrderManager<PriceType, OrderIdType> oms;
    OrderBook<PriceType, OrderIdType> book;
    TradeLogger<PriceType, OrderIdType> logger("trades.csv");
    MatchingEngine<PriceType, OrderIdType> engine;

    std::vector<long long> latencies;
    latencies.reserve(num_ticks);

    for (int i = 0; i < num_ticks; ++i) {
        Timer timer;
        timer.start();

        MarketData tick = feed.generate_tick();

        book.generate_order_from_tick(tick, oms);

        if (engine.match_top(book, oms)) {
            Trade<PriceType, OrderIdType> trade;
            trade.buy_id = 1;
            trade.sell_id = 2;
            trade.price = (tick.bid_price + tick.ask_price) / 2;
            trade.quantity = 50;
            trade.timestamp = std::chrono::high_resolution_clock::now();
            logger.log_trade(trade);
        }

        latencies.push_back(timer.stop());
    }

    logger.flush();

    analyzeLatencies(latencies);

    book.print_top_of_book();

    std::cout << "\nTrades have been logged to trades.csv \n";
    return 0;
}
