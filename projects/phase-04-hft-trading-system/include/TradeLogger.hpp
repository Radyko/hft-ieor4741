#pragma once
#include <vector>
#include <string>
#include <fstream>
#include "MatchingEngine.hpp"

template <typename PriceType, typename OrderIdType>
class TradeLogger {
public:
    TradeLogger(const std::string& filename);
    ~TradeLogger();

    void log_trade(const Trade<PriceType, OrderIdType>& trade);
    void flush();

private:
    std::vector<Trade<PriceType, OrderIdType>> trades_;
    std::ofstream file_;
};
