#ifndef ORDER_MANAGER_H
#define ORDER_MANAGER_H
#include <map>
#include <memory>

enum class Side { Buy, Sell };

enum class OrderStatus { New, Filled, PartiallyFilled, Cancelled };

struct MyOrder {
    int id;
    Side side;
    double price;
    int quantity;
    int filled = 0;
    OrderStatus status = OrderStatus::New;
};

class OrderManager
{
public:
    int place_order(Side side, double price, int qty);
    void cancel(int id);
    void handle_fill(int id, int filled_qty);
    void print_active_orders() const;
private:
    static int next_id_;
    std::map<int, std::unique_ptr<MyOrder>> orders;
};

#endif