#include "../include/OrderBook.hpp"

template <typename PriceType, typename OrderIdType>
OrderBook<PriceType, OrderIdType>::OrderBook()
    : rng_(std::random_device{}())
{}

template <typename PriceType, typename OrderIdType>
void OrderBook<PriceType, OrderIdType>::accept_order(OrderPtr order)
{
    if (order->is_buy)
        buy_orders.insert({order->price, order});
    else
        sell_orders.insert({order->price, order});
}

template <typename PriceType, typename OrderIdType>
void OrderBook<PriceType, OrderIdType>::generate_order_from_tick(
    const MarketData& tick,
    OrderManager<PriceType, OrderIdType>& oms)
{
    int q_buy = qty_dist_(rng_);
    int q_sell = qty_dist_(rng_);

    auto buy = oms.submit_new(tick.symbol, tick.bid_price, q_buy, true);
    auto sell = oms.submit_new(tick.symbol, tick.ask_price, q_sell, false);

    accept_order(buy);
    accept_order(sell);
}

template <typename PriceType, typename OrderIdType>
const typename OrderBook<PriceType, OrderIdType>::OrderT*
OrderBook<PriceType, OrderIdType>::best_bid() const {
    if (buy_orders.empty()) return nullptr;
    return buy_orders.rbegin()->second.get();
}

template <typename PriceType, typename OrderIdType>
const typename OrderBook<PriceType, OrderIdType>::OrderT*
OrderBook<PriceType, OrderIdType>::best_ask() const {
    if (sell_orders.empty()) return nullptr;
    return sell_orders.begin()->second.get();
}

template <typename PriceType, typename OrderIdType>
void OrderBook<PriceType, OrderIdType>::erase_by_id(OrderIdType id) {
    // remove matching order from either buy or sell side
    for (auto it = buy_orders.begin(); it != buy_orders.end(); ++it) {
        if (it->second->id == id) {
            buy_orders.erase(it);
            return;
        }
    }
    for (auto it = sell_orders.begin(); it != sell_orders.end(); ++it) {
        if (it->second->id == id) {
            sell_orders.erase(it);
            return;
        }
    }
}


template <typename PriceType, typename OrderIdType>
void OrderBook<PriceType, OrderIdType>::print_top_of_book() const
{
    if (!buy_orders.empty()) {
        auto best_bid = buy_orders.rbegin()->second.get();
        std::cout << "Best Bid: " << best_bid->price << " (" << best_bid->quantity << ")\n";
    }
    if (!sell_orders.empty()) {
        auto best_ask = sell_orders.begin()->second.get();
        std::cout << "Best Ask: " << best_ask->price << " (" << best_ask->quantity << ")\n";
    }
}

template class OrderBook<double, int>;
