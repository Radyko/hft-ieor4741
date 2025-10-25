// benchmark.cpp
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "include/Order.hpp"
#include "include/OrderBook.hpp"

static constexpr uint64_t N_EVENTS = 1'000'000;   // total events
static constexpr uint64_t BATCH    = 1000;         // events per batch for batch timing

// Exponential histogram buckets in nanoseconds: [0.5us, 1us, 2us, ..., ~1s]
static std::vector<uint64_t> make_ns_bucket_bounds() {
    std::vector<uint64_t> b;
    uint64_t ns = 500; // 0.5 us
    while (ns <= 1'000'000'000ULL) { // up to 1 second
        b.push_back(ns);
        ns *= 2;
    }
    return b;
}

static size_t bucket_index(uint64_t ns, const std::vector<uint64_t>& bounds) {
    // first bound >= ns
    for (size_t i = 0; i < bounds.size(); ++i) {
        if (ns <= bounds[i]) return i;
    }
    return bounds.size(); // overflow bucket
}

int main() {
    OrderBook book;

    // Random setup
    std::mt19937_64 rng(123456789);
    std::uniform_int_distribution<int> side_dist(0, 1);
    std::uniform_int_distribution<int> qty_dist(1, 100);
    std::uniform_int_distribution<int> price_ix_dist(0, 31);
    std::uniform_int_distribution<int> event_dist(0, 99); // 0..59=new, 60..89=amend, 90..99=delete

    // Discrete price ladder (double for simplicity)
    std::vector<Price> prices;
    prices.reserve(32);
    {
        // e.g., 100.00 to 103.10 step 0.10 (32 ticks)
        double p = 100.0;
        for (int i = 0; i < 32; ++i) { prices.push_back(p); p += 0.1; }
    }

    // Track active order ids so we can amend/delete real ones
    std::vector<id_t> active_ids;
    active_ids.reserve(2'000'000);
    id_t next_id = 1;

    // Timing storage
    using clock = std::chrono::high_resolution_clock;
    using ns    = std::chrono::nanoseconds;

    std::vector<uint64_t> per_event_ns;
    per_event_ns.resize(N_EVENTS);

    const auto bucket_bounds = make_ns_bucket_bounds();
    std::vector<uint64_t> histogram(bucket_bounds.size() + 1, 0); // + overflow

    uint64_t event_count = 0;
    uint64_t batch_count = 0;

    uint64_t total_batch_ns = 0;
    uint64_t min_ns = UINT64_MAX;
    uint64_t max_ns = 0;

    // Pre-create a distribution that uses active_ids size safely
    auto pick_active = [&](std::mt19937_64& gen) -> id_t {
        if (active_ids.empty()) return 0;
        std::uniform_int_distribution<size_t> pick(0, active_ids.size() - 1);
        return active_ids[pick(gen)];
    };

    auto batch_start = clock::now();

    while (event_count < N_EVENTS) {
        // Decide event type
        int r = event_dist(rng);
        bool do_new    = (r < 60);             // 60%
        bool do_amend  = (r >= 60 && r < 90);  // 30%
        bool do_delete = (r >= 90);            // 10%

        const auto t0 = clock::now();

        if (do_new) {
            // Create a new random order
            Order o;
            o.id       = next_id++;
            o.side     = (side_dist(rng) == 0) ? Side::Buy : Side::Sell;
            o.price    = prices[price_ix_dist(rng)];
            o.quantity = static_cast<Quantity>(qty_dist(rng));

            book.newOrder(o);
            active_ids.push_back(o.id);
        } else if (do_amend) {
            id_t id = pick_active(rng);
            if (id != 0) {
                Quantity newQty = static_cast<Quantity>(qty_dist(rng));
                // If amend fails (should be rare), ignore
                (void)book.amendOrder(id, newQty);
                // If newQty==0, your amend() routes to delete(), so remove from active_ids
                if (newQty == 0) {
                    // lazy removal: swap-erase when we hit a delete path or compact later
                    // keep it simple and skip; correctness not impacted for timing
                }
            } else {
                // No active orders; fall back to new
                Order o;
                o.id       = next_id++;
                o.side     = (side_dist(rng) == 0) ? Side::Buy : Side::Sell;
                o.price    = prices[price_ix_dist(rng)];
                o.quantity = static_cast<Quantity>(qty_dist(rng));
                book.newOrder(o);
                active_ids.push_back(o.id);
            }
        } else { // delete
            id_t id = pick_active(rng);
            if (id != 0) {
                bool ok = book.deleteOrder(id);
                if (ok) {
                    // Remove id from active_ids (swap-erase)
                    // Find position (linear scan is fine for simplicity)
                    for (size_t i = 0; i < active_ids.size(); ++i) {
                        if (active_ids[i] == id) {
                            active_ids[i] = active_ids.back();
                            active_ids.pop_back();
                            break;
                        }
                    }
                }
            } else {
                // No active ids; nothing to delete. Convert to new.
                Order o;
                o.id       = next_id++;
                o.side     = (side_dist(rng) == 0) ? Side::Buy : Side::Sell;
                o.price    = prices[price_ix_dist(rng)];
                o.quantity = static_cast<Quantity>(qty_dist(rng));
                book.newOrder(o);
                active_ids.push_back(o.id);
            }
        }

        const auto t1 = clock::now();
        const uint64_t dur = std::chrono::duration_cast<ns>(t1 - t0).count();

        per_event_ns[event_count] = dur;
        if (dur < min_ns) min_ns = dur;
        if (dur > max_ns) max_ns = dur;
        histogram[bucket_index(dur, bucket_bounds)]++;

        event_count++;

        // Batch timing
        if (event_count % BATCH == 0) {
            const auto batch_end = clock::now();
            const uint64_t bns = std::chrono::duration_cast<ns>(batch_end - batch_start).count();
            total_batch_ns += bns;
            batch_count++;
            batch_start = clock::now();
        }
    }

    // If last batch wasn't closed on an exact boundary, add it
    if (event_count % BATCH != 0) {
        const auto batch_end = clock::now();
        const uint64_t bns = std::chrono::duration_cast<ns>(batch_end - batch_start).count();
        total_batch_ns += bns;
        batch_count++;
    }

    // Compute totals/averages
    const long double total_ns = static_cast<long double>(total_batch_ns);
    const long double avg_ns   = total_ns / static_cast<long double>(N_EVENTS);

    // Median (copy indexes to avoid moving the huge vector)
    std::vector<uint64_t> tmp = per_event_ns;
    std::nth_element(tmp.begin(), tmp.begin() + tmp.size() / 2, tmp.end());
    const uint64_t median_ns = tmp[tmp.size() / 2];

    // Print report
    auto fmt_ns = [](uint64_t ns) -> std::string {
        // Simple human-ish formatting
        if (ns < 1'000ULL)               return std::to_string(ns) + " ns";
        else if (ns < 1'000'000ULL)      return std::to_string(ns / 1000ULL) + " us";
        else if (ns < 1'000'000'000ULL)  return std::to_string(ns / 1'000'000ULL) + " ms";
        else                              return std::to_string(ns / 1'000'000'000ULL) + " s";
    };

    std::cout << "==== OrderBook Benchmark ====\n";
    std::cout << "Events:   " << N_EVENTS << "\n";
    std::cout << "Batches:  " << batch_count << " (size " << BATCH << ")\n";
    std::cout << "Total:    " << fmt_ns(static_cast<uint64_t>(total_ns)) << "\n";
    std::cout << "Average:  " << avg_ns << " ns / event\n";
    std::cout << "Min:      " << min_ns << " ns\n";
    std::cout << "Median:   " << median_ns << " ns\n";
    std::cout << "Max:      " << max_ns << " ns\n\n";

    std::cout << "Histogram (per-event latency):\n";
    for (size_t i = 0; i < bucket_bounds.size(); ++i) {
        uint64_t upper = bucket_bounds[i];
        uint64_t count = histogram[i];
        std::cout << "  <= " << upper << " ns : " << count << "\n";
    }
    std::cout << "  >  " << bucket_bounds.back() << " ns : " << histogram.back() << "\n";

    return 0;
}
