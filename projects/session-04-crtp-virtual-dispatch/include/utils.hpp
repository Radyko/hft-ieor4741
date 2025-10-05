#pragma once
#include <chrono>
#include <cstdint>

// Prevent the optimizer from eliding computations.
// Works on GCC/Clang; MSVC falls back to a volatile sink.
template <class T>
inline void do_not_optimize_away(T const& value) {
#if defined(__GNUC__) || defined(__clang__)
    asm volatile("" : : "g"(value) : "memory");
#else
    volatile const void* sink = &value;
    (void)sink;
#endif
}

struct Timer {
    using clock = std::chrono::steady_clock;
    clock::time_point t0{};
    void start() { t0 = clock::now(); }
    double stop_ns() const {
        auto t1 = clock::now();
        return std::chrono::duration<double, std::nano>(t1 - t0).count();
    }
};

// Simple, fast xorshift32 PRNG (deterministic)
struct XorShift32 {
    std::uint32_t state;
    explicit XorShift32(std::uint32_t seed = 0xDEADBEEF) : state(seed) {}

    std::uint32_t next_u32() {
        std::uint32_t x = state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state = x;
        return x;
    }

    // Produce doubles in [low, high)
    double uniform(double low, double high) {
        const double scale = 1.0 / static_cast<double>(1u << 24);
        double u = static_cast<double>(next_u32() & 0xFFFFFFu) * scale;
        return low + (high - low) * u;
    }
};
