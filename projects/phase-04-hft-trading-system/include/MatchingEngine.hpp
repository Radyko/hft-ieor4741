#pragma once
#include <functional>
#include <chrono>

template <typename PriceType, typename OrderIdType> class OrderBook;
template <typename PriceType, typename OrderIdType> class OrderManager;
template <typename PriceType, typename OrderIdType> struct Order;

template <typename PriceType, typename OrderIdType>
struct Trade {
    OrderIdType buy_id{};
    OrderIdType sell_id{};
    PriceType   price{};
    int         quantity{};
    std::chrono::high_resolution_clock::time_point timestamp{};
};

template <typename PriceType, typename OrderIdType>
class MatchingEngine {
public:
    using OrderT       = Order<PriceType, OrderIdType>;
    using TradeT       = Trade<PriceType, OrderIdType>;
    using TradeHandler = std::function<void(const TradeT&)>;

    explicit MatchingEngine(TradeHandler on_trade = nullptr);
    ~MatchingEngine() = default;

    bool match_top(OrderBook<PriceType, OrderIdType>& book,
                   OrderManager<PriceType, OrderIdType>& oms);

    void match_while_crossed(OrderBook<PriceType, OrderIdType>& book,
                             OrderManager<PriceType, OrderIdType>& oms);

    void set_trade_handler(TradeHandler cb);

private:
    TradeHandler on_trade_;
};
