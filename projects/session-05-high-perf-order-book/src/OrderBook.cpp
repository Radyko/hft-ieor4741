#include "../include/OrderBook.hpp"

void OrderBook::newOrder(Order const& o)
{
    auto& levels     = (o.side == Side::Buy) ? bids_ : asks_;
    auto& free_list  = (o.side == Side::Buy) ? free_bid_levels_ : free_ask_levels_;
    auto& price_map  = (o.side == Side::Buy) ? price_to_bid_level_ : price_to_ask_level_;
    int&  best_idx   = (o.side == Side::Buy) ? best_bid_idx_ : best_ask_idx_;

    size_t levelIdx;
    auto it = price_map.find(o.price);
    if (it != price_map.end()) {
        levelIdx = it->second;
    } else {
        if (!free_list.empty()) {
            levelIdx = free_list.back();
            free_list.pop_back();
            levels[levelIdx] = PriceLevel(o.side, o.price);
        } else {
            levelIdx = levels.size();
            levels.emplace_back(o.side, o.price);
        }
        price_map[o.price] = levelIdx;

        if (best_idx < 0) {
            best_idx = static_cast<int>(levelIdx);
        } else {
            Price best_price = levels[static_cast<size_t>(best_idx)].price;
            bool better = (o.side == Side::Buy) ? (o.price > best_price) : (o.price < best_price);
            if (better) best_idx = static_cast<int>(levelIdx);
        }
    }

    PriceLevel& lvl = levels[levelIdx];
    size_t orderIdx = lvl.orders.size();
    lvl.orders.push_back(o);
    lvl.total_qty += o.quantity;

    id_to_index_[o.id] = { o.side, levelIdx, orderIdx };

}

bool OrderBook::amendOrder(id_t id, Quantity newQty)
{
    auto it = id_to_index_.find(id);
    if (it == id_to_index_.end()) return false;

    Index loc = it->second;
    auto& levels = (loc.side == Side::Buy) ? bids_ : asks_;
    if (loc.levelIdx >= levels.size()) return false;

    PriceLevel& lvl = levels[loc.levelIdx];
    if (!lvl.active) return false;
    if (loc.orderIdx >= lvl.orders.size()) return false;

    Order& ord = lvl.orders[loc.orderIdx];

    if (newQty > ord.quantity) lvl.total_qty += (newQty - ord.quantity);
    else                       lvl.total_qty -= (ord.quantity - newQty);

    ord.quantity = newQty;

    if (ord.quantity == 0) {
        return deleteOrder(id);
    }
    return true;
}

bool OrderBook::deleteOrder(id_t id)
{
    auto it = id_to_index_.find(id);
    if (it == id_to_index_.end()) return false;

    Index loc = it->second;

    auto& levels    = (loc.side == Side::Buy) ? bids_ : asks_;
    auto& free_list = (loc.side == Side::Buy) ? free_bid_levels_ : free_ask_levels_;
    auto& price_map = (loc.side == Side::Buy) ? price_to_bid_level_ : price_to_ask_level_;
    int&  best_idx  = (loc.side == Side::Buy) ? best_bid_idx_ : best_ask_idx_;

    if (loc.levelIdx >= levels.size()) return false;
    PriceLevel& lvl = levels[loc.levelIdx];
    if (!lvl.active) return false;
    if (loc.orderIdx >= lvl.orders.size()) return false;

    Quantity q = lvl.orders[loc.orderIdx].quantity;
    if (lvl.total_qty >= q) lvl.total_qty -= q; else lvl.total_qty = 0;

    size_t last = lvl.orders.size() - 1;
    if (loc.orderIdx != last) {
        Order moved = lvl.orders[last];
        lvl.orders[loc.orderIdx] = moved;

        auto mit = id_to_index_.find(moved.id);
        if (mit != id_to_index_.end()) {
            mit->second.orderIdx = loc.orderIdx;
        }
    }
    lvl.orders.pop_back();

    id_to_index_.erase(it);

    if (lvl.orders.empty() || lvl.total_qty == 0) {
        lvl.orders.clear();
        lvl.total_qty = 0;
        lvl.active = false;

        auto pit = price_map.find(lvl.price);
        if (pit != price_map.end() && pit->second == loc.levelIdx) {
            price_map.erase(pit);
        }

        free_list.push_back(loc.levelIdx);

        if (best_idx == static_cast<int>(loc.levelIdx)) {
            int new_best = -1;
            Price best_price = 0.0;
            bool first = true;
            for (size_t i = 0; i < levels.size(); ++i) {
                if (!levels[i].active) continue;
                if (levels[i].total_qty == 0 || levels[i].orders.empty()) continue;
                if (first) {
                    first = false;
                    new_best = static_cast<int>(i);
                    best_price = levels[i].price;
                } else {
                    bool better = (loc.side == Side::Buy) ? (levels[i].price > best_price)
                                                          : (levels[i].price < best_price);
                    if (better) {
                        new_best = static_cast<int>(i);
                        best_price = levels[i].price;
                    }
                }
            }
            best_idx = new_best;
        }
    }

    return true;
}

PriceLevel OrderBook::topOfBook(Side s) const
{
    const auto& levels = (s == Side::Buy) ? bids_ : asks_;
    int best_idx = (s == Side::Buy) ? best_bid_idx_ : best_ask_idx_;

    if (best_idx >= 0 && static_cast<size_t>(best_idx) < levels.size()) {
        const PriceLevel& lvl = levels[static_cast<size_t>(best_idx)];
        if (lvl.active && lvl.total_qty > 0 && !lvl.orders.empty()) {
            return lvl; // okay
        }
    }

    int new_best = -1;
    Price best_price = 0.0;
    bool first = true;
    for (size_t i = 0; i < levels.size(); ++i) {
        if (!levels[i].active) continue;
        if (levels[i].total_qty == 0 || levels[i].orders.empty()) continue;
        if (first) {
            first = false;
            new_best = static_cast<int>(i);
            best_price = levels[i].price;
        } else {
            bool better = (s == Side::Buy) ? (levels[i].price > best_price)
                                           : (levels[i].price < best_price);
            if (better) {
                new_best = static_cast<int>(i);
                best_price = levels[i].price;
            }
        }
    }

    if (new_best >= 0) {
        return levels[static_cast<size_t>(new_best)];
    }

    return PriceLevel();
}

size_t OrderBook::orderCount(Price p, Side s) const
{
    const auto& price_map = (s == Side::Buy) ? price_to_bid_level_ : price_to_ask_level_;
    auto it = price_map.find(p);
    if (it == price_map.end()) return 0;

    const auto& levels = (s == Side::Buy) ? bids_ : asks_;
    size_t idx = it->second;
    if (idx >= levels.size()) return 0;

    const PriceLevel& lvl = levels[idx];
    if (!lvl.active) return 0;
    return lvl.orders.size();
}

Quantity OrderBook::totalVolume(Price p, Side s) const
{
    const auto& price_map = (s == Side::Buy) ? price_to_bid_level_ : price_to_ask_level_;
    auto it = price_map.find(p);
    if (it == price_map.end()) return 0;

    const auto& levels = (s == Side::Buy) ? bids_ : asks_;
    size_t idx = it->second;
    if (idx >= levels.size()) return 0;

    const PriceLevel& lvl = levels[idx];
    if (!lvl.active) return 0;
    return lvl.total_qty;
}
