#pragma once
#include <cstdint>

using Price = double;
using Quantity = std::uint64_t;

enum class Side { Buy, Sell };

struct Order {
    id_t id;
    Price price;
    Quantity quantity;
    Side side;
};
