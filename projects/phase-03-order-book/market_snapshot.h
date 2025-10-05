#ifndef MARKET_SNAPSHOT_H
#define MARKET_SNAPSHOT_H

#include <map>
#include <memory>

struct PriceLevel {
    double price;
    int quantity;

    PriceLevel(double p, int q) : price(p), quantity(q) {}
};

class MarketSnapshot
{
public:
    void update_bid(double price, int qty);
    void update_ask(double price, int qty);
    const PriceLevel* get_best_bid() const;
    const PriceLevel* get_best_ask() const;

private:
    std::map<double, std::unique_ptr<PriceLevel>, std::greater<>> bids; // sorted descending
    std::map<double, std::unique_ptr<PriceLevel>> asks; // sorted ascending
};



#endif //MARKET_SNAPSHOT_H
