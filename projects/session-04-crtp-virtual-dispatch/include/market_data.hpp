#pragma once

// Basic quote type
struct Quote {
    double bid{};
    double ask{};
    double bid_qty{};
    double ask_qty{};
};

// Pure math helpers (likely to inline)
inline double mid(const Quote& q) noexcept {
    return (q.bid + q.ask) * 0.5;
}

inline double microprice(const Quote& q) noexcept {
    const double denom = q.bid_qty + q.ask_qty;
    // If denom == 0, fall back to mid to avoid NaN.
    return denom > 0.0
        ? (q.bid * q.ask_qty + q.ask * q.bid_qty) / denom
        : mid(q);
}

inline double imbalance(const Quote& q) noexcept {
    const double denom = q.bid_qty + q.ask_qty;
    // If denom == 0, define imbalance as 0.
    return denom > 0.0
        ? (q.bid_qty - q.ask_qty) / denom
        : 0.0;
}
