#include "../include/MarketData.hpp"
#include <random>

GenerateMarketData::GenerateMarketData()
    : gen(std::random_device{}()),
      apple_price(200, 230),
      apple_spread(1, 5)
{}

MarketData GenerateMarketData::generate_tick()
{
    MarketData tick;
    tick.symbol = "AAPL";

    int base = apple_price(gen);
    int spread = apple_spread(gen);

    tick.bid_price = base - spread;
    tick.ask_price = base + spread;
    tick.timestamp = std::chrono::high_resolution_clock::now();

    return tick;
}
