#pragma once
#include <vector>
#include "Order.hpp"

struct alignas(64) PriceLevel {
    Side side{};
    Price price{};
    Quantity total_qty{};
    bool active{false};
    std::vector<Order> orders;

    PriceLevel() = default;
    PriceLevel(Side s, Price p)
        : side(s), price(p), total_qty(0), active(true), orders() {}
};