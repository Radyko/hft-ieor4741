#pragma once
#include "market_data.hpp"

// Base interface using virtual dispatch.
struct StrategyVirtual {
    virtual ~StrategyVirtual() = default;

    // Return a signal (e.g., alpha) for a given Quote.
    virtual double on_tick(const Quote& q) = 0;
};

// Concrete implementation that matches SignalStrategyCRTP’s behavior.
struct SignalStrategyVirtual final : StrategyVirtual {
    double alpha1;
    double alpha2;

    explicit SignalStrategyVirtual(double a1, double a2)
        : alpha1(a1), alpha2(a2) {}

    double on_tick(const Quote& q) override {
        const double mp  = microprice(q);
        const double m   = mid(q);
        const double imb = imbalance(q);
        return alpha1 * (mp - m) + alpha2 * imb;
    }
};
