#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <chrono>
#include <vector>
#include <cassert>
#include <random>

struct Order {
    std::string id;
    double price;
    int quantity;
    bool isBuy;
};

class OrderBook {
public:
    // keep data members public only if you need direct test access
    std::map<double, std::unordered_map<std::string, Order>> orderLevels;
    std::unordered_map<std::string, Order> orderLookup;

    void addOrder(const std::string& id, double price, int quantity, bool isBuy) {
        Order order{ id, price, quantity, isBuy };
        orderLevels[price][id] = order;
        orderLookup[id] = order;
    }

    void modifyOrder(const std::string& id, double newPrice, int newQuantity) {
        auto it = orderLookup.find(id);
        if (it != orderLookup.end()) {
            Order oldOrder = it->second;
            orderLevels[oldOrder.price].erase(id);
            addOrder(id, newPrice, newQuantity, oldOrder.isBuy);
        }
    }

    void deleteOrder(const std::string& id) {
        auto it = orderLookup.find(id);
        if (it != orderLookup.end()) {
            Order order = it->second;
            orderLevels[order.price].erase(id);
            orderLookup.erase(it);
        }
    }
};

// --- loop-unrolled bulk handler (factor 2, matching your snippet) ---
inline void handleOrder(OrderBook& book, const Order& o) {
    book.addOrder(o.id, o.price, o.quantity, o.isBuy);
}

void processOrders(std::vector<Order>& orders, OrderBook& book) {
    for (size_t i = 0; i < orders.size(); i += 2) {  // Unrolling by 2
        handleOrder(book, orders[i]);
        if (i + 1 < orders.size()) handleOrder(book, orders[i + 1]);
    }
}

// --- your unit test ---
void testAddOrder() {
    OrderBook book;
    book.addOrder("ORD001", 50.10, 100, true);
    assert(book.orderLookup.count("ORD001") == 1);  // Order should exist
}

// --- your stress test helper (adds random orders) ---
void stressTest(OrderBook& book, int numOrders) {
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> priceDist(50.0, 100.0);
    std::uniform_int_distribution<int> quantityDist(1, 500);

    std::vector<Order> batch;
    batch.reserve(numOrders);
    for (int i = 0; i < numOrders; ++i) {
        batch.push_back(Order{
            "ORD" + std::to_string(i),
            priceDist(rng),
            quantityDist(rng),
            true
        });
    }

    // use the unrolled bulk path
    processOrders(batch, book);
}

// --- tiny driver that times different scales and prints results you can paste into Python ---
int main() {
    testAddOrder();

    std::vector<int> sizes = {1000, 5000, 10000, 50000, 100000};
    std::vector<double> times;

    for (int n : sizes) {
        OrderBook ob;
        auto t0 = std::chrono::high_resolution_clock::now();
        stressTest(ob, n);
        auto t1 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> dt = t1 - t0;
        times.push_back(dt.count());
        std::cout << n << "," << dt.count() << "\n"; // CSV line: size,time_seconds
    }
    return 0;
}
