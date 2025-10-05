#include "order_manager.h"
#include <iostream>
#include <memory>

int OrderManager::next_id_ = 1;

int OrderManager::place_order(Side side, double price, int qty)
{
    int id = next_id_++;
    orders.emplace_hint(
        orders.end(),
        id,
        std::make_unique<MyOrder>(MyOrder{id, side, price, qty}));

    return id;
}

void OrderManager::print_active_orders() const
{
    std::cout << "Active orders:\n";
    for (const auto& [id, orderPtr] : orders)
    {
        const MyOrder& o = *orderPtr;
        if (o.status == OrderStatus::Filled || o.status == OrderStatus::Cancelled)
            continue;

        std::cout << "ID " << id
                  << " | Side: " << (o.side == Side::Buy ? "Buy" : "Sell")
                  << " | Price: " << o.price
                  << " | Qty: " << o.quantity
                  << " | Filled: " << o.filled
                  << " | Status: ";

        switch (o.status) {
        case OrderStatus::New: std::cout << "New"; break;
        case OrderStatus::Filled: std::cout << "Filled"; break;
        case OrderStatus::PartiallyFilled: std::cout << "PartiallyFilled"; break;
        case OrderStatus::Cancelled: std::cout << "Cancelled"; break;
        }

        std::cout << '\n';
    }
}

void OrderManager::cancel(int id)
{
    if (orders.find(id) != orders.end())
    {
        orders.erase(id);
    }
}

void OrderManager::handle_fill(int id, int filled_qty)
{
    auto it = orders.find(id);
    if (it == orders.end()) {
        std::cout << "Order " << id << " not found.\n";
        return;
    }

    MyOrder& order = *it->second;

    if (order.status == OrderStatus::Filled || order.status == OrderStatus::Cancelled) {
        std::cout << "Order " << id << " already closed.\n";
        return;
    }

    if (filled_qty <= 0) return;

    int remaining = order.quantity - order.filled;

    int delta = std::min(filled_qty, remaining);
    order.filled += delta;

    if (order.filled >= order.quantity) {
        order.status = OrderStatus::Filled;
        std::cout << "Order " << id << " fully filled (" << order.quantity << ").\n";
    } else {
        order.status = OrderStatus::PartiallyFilled;
        std::cout << "Order " << id << " partially filled ("
                  << order.filled << "/" << order.quantity << ").\n";
    }}

