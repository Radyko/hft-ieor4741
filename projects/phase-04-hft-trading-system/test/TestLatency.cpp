#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

void analyzeLatencies(std::vector<long long> latencies) {
    if (latencies.empty()) {
        std::cout << "No latency data available.\n";
        return;
    }

    std::sort(latencies.begin(), latencies.end());

    auto min = latencies.front();
    auto max = latencies.back();
    double mean = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();

    double variance = 0.0;
    for (auto l : latencies)
        variance += (l - mean) * (l - mean);
    double stddev = std::sqrt(variance / latencies.size());

    // 99th percentile (simple rank-based)
    size_t idx = static_cast<size_t>(latencies.size() * 0.99);
    if (idx >= latencies.size()) idx = latencies.size() - 1;
    long long p99 = latencies[idx];

    std::cout << "Tick-to-Trade Latency (nanoseconds):\n";
    std::cout << "Min: " << min << "\nMax: " << max << "\nMean: " << mean
              << "\nStdDev: " << stddev << "\nP99: " << p99 << "\n";
}
