#pragma once
#include <unordered_map>
#include <vector>
#include "PriceLevel.hpp"

// Where an order lives
struct Index {
    Side   side{};
    size_t levelIdx{};
    size_t orderIdx{};
};

class OrderBook {
public:
    // Core ops
    void newOrder(Order const& o);
    bool amendOrder(id_t id, Quantity newQty);   // quantity-only
    bool deleteOrder(id_t id);

    // Queries
    PriceLevel topOfBook(Side s) const;          // returns by value; inactive if none
    size_t     orderCount(Price p, Side s) const;
    Quantity   totalVolume(Price p, Side s) const;

private:
    // Unsorted per-side level storage. We NEVER erase in the middle.
    std::vector<PriceLevel> bids_;
    std::vector<PriceLevel> asks_;

    // Free lists of inactive level slots we can reuse
    std::vector<size_t> free_bid_levels_;
    std::vector<size_t> free_ask_levels_;

    // price -> level index (active levels only)
    std::unordered_map<Price, size_t> price_to_bid_level_;
    std::unordered_map<Price, size_t> price_to_ask_level_;

    // id -> location
    std::unordered_map<id_t, Index> id_to_index_;

    // Best-of-book indices (-1 means none)
    int best_bid_idx_{-1};
    int best_ask_idx_{-1};
};
