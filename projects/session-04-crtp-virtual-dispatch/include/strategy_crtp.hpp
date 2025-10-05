#pragma once
#include "market_data.hpp"

// CRTP base: static (non-virtual) dispatch.
// Derived must implement: double on_tick_impl(const Quote&)
template <typename Derived>
struct StrategyBase {
    double on_tick(const Quote& q) const {
        return static_cast<const Derived*>(this)->on_tick_impl(q);
    }
};

// Same behavior as the virtual version, but via CRTP.
struct SignalStrategyCRTP : StrategyBase<SignalStrategyCRTP> {
    double alpha1;
    double alpha2;

    explicit SignalStrategyCRTP(double a1, double a2)
        : alpha1(a1), alpha2(a2) {}

    double on_tick_impl(const Quote& q) const {
        const double mp  = microprice(q);
        const double m   = mid(q);
        const double imb = imbalance(q);
        return alpha1 * (mp - m) + alpha2 * imb;
    }
};
