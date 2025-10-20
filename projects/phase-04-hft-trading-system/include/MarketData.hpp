#pragma once
#include <string>
#include <vector>
#include <random>
#include <chrono>

struct alignas(64) MarketData {
    std::string symbol;
    double bid_price;
    double ask_price;
    std::chrono::high_resolution_clock::time_point timestamp;
};

class GenerateMarketData
{
public:
    GenerateMarketData();
    ~GenerateMarketData() = default;

    MarketData generate_tick();

private:
    std::mt19937 gen;
    std::uniform_int_distribution<> apple_price;
    std::uniform_int_distribution<> apple_spread;
};
