#pragma once
#include <unordered_map>
#include <memory>
#include <atomic>
#include "Order.hpp"

template <typename PriceType, typename OrderIdType>
class OrderManager {
public:
    using OrderT = Order<PriceType, OrderIdType>;
    using OrderPtr = std::shared_ptr<OrderT>;

    OrderManager();
    ~OrderManager() = default;

    OrderPtr submit_new(const std::string& symbol, PriceType price, int qty, bool is_buy);
    bool request_cancel(OrderIdType id);
    void apply_fill(OrderIdType id, int filled_qty);
    OrderPtr get(OrderIdType id) const;

private:
    std::atomic<OrderIdType> next_id_{0};
    std::unordered_map<OrderIdType, OrderPtr> orders_;
};
