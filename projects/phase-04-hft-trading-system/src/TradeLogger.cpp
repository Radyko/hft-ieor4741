#include "../include/TradeLogger.hpp"
#include <chrono>

template <typename PriceType, typename OrderIdType>
TradeLogger<PriceType, OrderIdType>::TradeLogger(const std::string& filename) {
    file_.open(filename);
    file_ << "timestamp_ns,buy_id,sell_id,price,quantity\n";
    trades_.reserve(1000);
}

template <typename PriceType, typename OrderIdType>
TradeLogger<PriceType, OrderIdType>::~TradeLogger() {
    flush();
    if (file_.is_open())
        file_.close();
}

template <typename PriceType, typename OrderIdType>
void TradeLogger<PriceType, OrderIdType>::log_trade(const Trade<PriceType, OrderIdType>& trade) {
    trades_.push_back(trade);

    if (trades_.size() >= 100) {
        flush();
    }
}

template <typename PriceType, typename OrderIdType>
void TradeLogger<PriceType, OrderIdType>::flush() {
    for (const auto& tr : trades_) {
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                      tr.timestamp.time_since_epoch()).count();

        file_ << ns << ","
              << tr.buy_id << ","
              << tr.sell_id << ","
              << tr.price << ","
              << tr.quantity << "\n";
    }
    trades_.clear();
    file_.flush();
}

template class TradeLogger<double, int>;
