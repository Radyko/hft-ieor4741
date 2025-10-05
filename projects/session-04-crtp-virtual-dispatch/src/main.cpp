#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <vector>

#include "market_data.hpp"
#include "utils.hpp"
#include "strategy_virtual.hpp"  // StrategyVirtual / SignalStrategyVirtual
#include "strategy_crtp.hpp"     // StrategyBase<>, SignalStrategyCRTP

// Free function baseline (control)
inline double signal_free(const Quote& q, double a1, double a2) {
    const double mp  = microprice(q);
    const double m   = mid(q);
    const double imb = imbalance(q);
    return a1 * (mp - m) + a2 * imb;
}

static void generate_ticks(std::vector<Quote>& out, std::uint32_t n, std::uint32_t seed) {
    out.resize(n);
    XorShift32 rng(seed);
    for (std::uint32_t i = 0; i < n; ++i) {
        double midp = rng.uniform(99.5, 100.5);   // around $100
        double sprd = rng.uniform(0.0005, 0.02);  // 0.05–2 cents
        double bq   = rng.uniform(100.0, 5000.0); // sizes
        double aq   = rng.uniform(100.0, 5000.0);
        out[i] = Quote{
            midp - 0.5 * sprd,
            midp + 0.5 * sprd,
            bq,
            aq
        };
    }
}

// Prevent dead code elimination by accumulating a checksum
template <typename F>
static double run_bench(const char* name, const std::vector<Quote>& ticks, F&& func, int iters) {
    Timer t; t.start();
    double sink = 0.0;

    for (int r = 0; r < iters; ++r) {
        for (const auto& q : ticks) {
            double s = func(q);
            sink += s * 1e-9; // keep it small
        }
    }

    do_not_optimize_away(sink);

    double ns = t.stop_ns();
    std::printf("%-18s  time: %.3f ms  sink=%.6f\n", name, ns / 1e6, sink);
    return ns;
}

int main(int argc, char** argv) {
    // Parameters (can be overridden from CLI)
    std::uint32_t n_ticks = 10'000'000; // 10M
    int iters = 1;
    double a1 = 0.75;
    double a2 = 0.25;
    std::uint32_t seed = 0xC001D00D;

    if (argc > 1) n_ticks = static_cast<std::uint32_t>(std::strtoul(argv[1], nullptr, 10));
    if (argc > 2) iters   = std::atoi(argv[2]);

    std::cout << "Generating " << n_ticks << " ticks, iters=" << iters << "...\n";
    std::vector<Quote> ticks;
    generate_ticks(ticks, n_ticks, seed);

    // Baseline (free function)
    auto ns_free = run_bench("free_function", ticks,
        [=](const Quote& q) { return signal_free(q, a1, a2); }, iters);

    // Runtime polymorphism (virtual)
    SignalStrategyVirtual virt(a1, a2);
    StrategyVirtual* s = &virt; // If your base is named IStrategy, change this type accordingly.
    auto ns_virtual = run_bench("virtual_call", ticks,
        [=](const Quote& q) { return s->on_tick(q); }, iters);

    // CRTP static polymorphism
    SignalStrategyCRTP crtp(a1, a2);
    auto ns_crtp = run_bench("crtp_call", ticks,
        [=](const Quote& q) { return crtp.on_tick(q); }, iters);

    const double total_ops = static_cast<double>(n_ticks) * iters;

    auto report = [&](const char* name, double ns) {
        double ns_per_tick = ns / total_ops;
        double tps = 1e9 / ns_per_tick;
        std::printf("%-18s  ns/tick: %.3f  ticks/sec: %.2f M\n",
                    name, ns_per_tick, tps / 1e6);
    };

    std::puts("\n=== Summary ===");
    report("free_function", ns_free);
    report("virtual_call", ns_virtual);
    report("crtp_call", ns_crtp);

    std::puts("\nTip: run `perf stat -e cycles,instructions,branches,branch-misses ./hft [N] [iters]` on Linux.");
    return 0;
}
