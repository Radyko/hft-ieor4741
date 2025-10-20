#pragma once
#include <map>
#include <memory>
#include <random>
#include <iostream>
#include "Order.hpp"
#include "MarketData.hpp"
#include "OrderManager.hpp"

template <typename PriceType, typename OrderIdType>
class OrderBook {
public:
    using OrderT = Order<PriceType, OrderIdType>;
    using OrderPtr = std::shared_ptr<OrderT>;

    OrderBook();
    ~OrderBook() = default;

    void accept_order(OrderPtr order);
    void generate_order_from_tick(const MarketData& tick, OrderManager<PriceType, OrderIdType>& oms);
    void print_top_of_book() const;

    const OrderT* best_bid() const;
    const OrderT* best_ask() const;
    void erase_by_id(OrderIdType id);

private:
    std::mt19937 rng_;
    std::uniform_int_distribution<int> qty_dist_{5, 100};

    std::multimap<PriceType, OrderPtr> buy_orders;
    std::multimap<PriceType, OrderPtr> sell_orders;
};
