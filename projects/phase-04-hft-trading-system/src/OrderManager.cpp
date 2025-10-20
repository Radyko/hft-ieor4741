#include "../include/OrderManager.hpp"

template <typename PriceType, typename OrderIdType>
OrderManager<PriceType, OrderIdType>::OrderManager()
    : next_id_(0)
{}

template <typename PriceType, typename OrderIdType>
auto OrderManager<PriceType, OrderIdType>::submit_new(const std::string& symbol, PriceType price, int qty, bool is_buy)
    -> OrderPtr
{
    auto order = std::make_shared<OrderT>(next_id_++, symbol, price, qty, is_buy);
    orders_[order->id] = order;
    return order;
}

template <typename PriceType, typename OrderIdType>
bool OrderManager<PriceType, OrderIdType>::request_cancel(OrderIdType id)
{
    auto it = orders_.find(id);
    if (it == orders_.end()) return false;

    auto& ord = it->second;
    if (ord->state == OrderState::Filled || ord->state == OrderState::Canceled)
        return false;

    ord->state = OrderState::Canceled;
    ord->remaining = 0;
    return true;
}

template <typename PriceType, typename OrderIdType>
void OrderManager<PriceType, OrderIdType>::apply_fill(OrderIdType id, int filled_qty)
{
    auto it = orders_.find(id);
    if (it == orders_.end()) return;

    auto& ord = it->second;
    if (ord->state == OrderState::Canceled || ord->state == OrderState::Filled)
        return;

    ord->remaining -= filled_qty;
    if (ord->remaining <= 0) {
        ord->remaining = 0;
        ord->state = OrderState::Filled;
    } else {
        ord->state = OrderState::PartiallyFilled;
    }
}

template <typename PriceType, typename OrderIdType>
auto OrderManager<PriceType, OrderIdType>::get(OrderIdType id) const -> OrderPtr
{
    auto it = orders_.find(id);
    return (it != orders_.end()) ? it->second : nullptr;
}

template class OrderManager<double, int>;
