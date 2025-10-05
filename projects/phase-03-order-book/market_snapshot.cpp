#include "market_snapshot.h"
#include <memory>  // for std::make_unique

// Define methods as belonging to MarketSnapshot (use the scope resolution operator ::)
void MarketSnapshot::update_bid(double price, int qty)
{
    auto it = bids.find(price);
    if (it == bids.end())
    {
        bids.emplace(price, std::make_unique<PriceLevel>(price, qty));
    } else
    {
        it->second->quantity += qty;
    }
}

void MarketSnapshot::update_ask(double price, int qty)
{
    auto it = asks.find(price);
    if (it == asks.end())
    {
        asks.emplace(price, std::make_unique<PriceLevel>(price, qty));
    } else
    {
        it->second->quantity += qty;
    }
}

// These are const because they don’t modify the object
const PriceLevel* MarketSnapshot::get_best_bid() const
{
    if (bids.empty()) return nullptr;
    return bids.begin()->second.get();
}

const PriceLevel* MarketSnapshot::get_best_ask() const
{
    if (asks.empty()) return nullptr;
    return asks.begin()->second.get();
}
