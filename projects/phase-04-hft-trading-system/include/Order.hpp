#pragma once
#include <string>
#include <type_traits>

enum class OrderState {
    New,
    PartiallyFilled,
    Filled,
    Canceled
};

template <typename PriceType, typename OrderIdType>
struct Order {
    static_assert(std::is_integral<OrderIdType>::value, "Order ID must be an integer");

    OrderIdType id;
    std::string symbol;
    PriceType price;
    int quantity;
    int remaining;
    bool is_buy;
    OrderState state;

    Order(OrderIdType id_,
          std::string sym,
          PriceType pr,
          int qty,
          bool buy)
        : id(id_),
          symbol(std::move(sym)),
          price(pr),
          quantity(qty),
          remaining(qty),
          is_buy(buy),
          state(OrderState::New)
    {}
};
