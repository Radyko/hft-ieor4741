#include <cstring>
#include <vector>
#include <random>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <algorithm>

const uint64_t ORDERS_DEFAULT = 20000000;
const int REPEATS_DEFAULT = 10;

static volatile uint64_t g_sink = 0;

void write_csv_header(FILE* out) {
    std::fprintf(out, "pattern,impl,repeat,orders,elapsed_ns,ops_per_sec,checksum\n");
}


static uint64_t Book1[64];
static uint64_t Book2[64];
static int counters[32];

void reset_state()
{
    std::memset(Book1, 0, sizeof(Book1));
    std::memset(Book2, 0, sizeof(Book2));
    std::memset(counters, 0, sizeof(counters));
}

struct Order
{
    uint64_t id;
    int side;
    int qty;
    int price;
    int payload[2];
};

std::vector<Order> make_orders(size_t N, uint64_t seed) {
    std::vector<Order> v;
    v.resize(N);
    std::mt19937_64 rng(seed); //deterministic seeded random number

    std::uniform_int_distribution<int> side01(0,1);
    std::uniform_int_distribution<int> qtyDist(1,1000);
    std::uniform_int_distribution<int> priceDist(90,120);

    for (size_t i = 0; i < N; ++i) {
        Order o;
        o.id = static_cast<uint64_t>(i);
        o.side = side01(rng);
        o.qty = qtyDist(rng);
        o.price = priceDist(rng);
        o.payload[0] = 0;
        o.payload[1] = 0;
        v[i] = o;
    }
    return v;
}

// Strategy labels
enum class Strat : uint8_t { A = 0, B = 1 };
enum class Pattern : uint8_t { HomogeneousA = 0, Mixed50 = 1, Bursty64_16 = 2 };

// Homogeneous: everyone → A
std::vector<Strat> make_assignments_homogeneous(size_t N) {
    return std::vector<Strat>(N, Strat::A);
}

// Mixed 50/50 with deterministic seed
std::vector<Strat> make_assignments_mixed50(size_t N, uint64_t seed) {
    std::vector<Strat> assignments;
    assignments.reserve(N); // avoid reallocations
    std::mt19937_64 rng(seed);
    std::bernoulli_distribution coin(0.5);
    for (size_t i = 0; i < N; ++i)
        assignments.push_back(coin(rng) ? Strat::A : Strat::B);
    return assignments;
}

// Bursty: 64 A then 16 B, repeat
std::vector<Strat> make_assignments_bursty(size_t N) {
    std::vector<Strat> assignments;
    assignments.reserve(N);
    const int runA = 64, runB = 16;
    while (assignments.size() < N) {
        for (int i = 0; i < runA && assignments.size() < N; ++i)
            assignments.push_back(Strat::A);
        for (int i = 0; i < runB && assignments.size() < N; ++i)
            assignments.push_back(Strat::B);
    }
    return assignments;
}

uint64_t workA(Order& o)
{
    int p = (o.price + (o.qty ^ (o.side * 7)));
    int q = (o.qty + 3) * 2 - (o.side ? 1 : 0);
    int index = (int)((o.id + p) & 63);
    int cix = (p + q) & 31;
    if ((p & 1) == 0)
    {
        counters[cix] += 1;
    } else
    {
        counters[cix] -= 1;
    };
    Book1[index] = (Book1[index] ^ (uint64_t)p);
    Book2[index] = (Book2[index] + (uint64_t)q);
    o.payload[0] = p;
    o.payload[1] = q;
    return (Book1[index] + (Book2[index] << 1)) ^ (uint64_t)(counters[cix] & 0xffff);

};

uint64_t workB(Order& o)
{
    int p = (o.price + (o.qty ^ (o.side * 4)));
    int q = (o.qty + 1) * 2 - (o.side ? 1 : 0);
    int index = (int)((o.id + p) & 63);   // 0..63
    int cix = (p + q) & 31;
    if ((p & 1) == 0)
    {
        counters[cix] += 1;
    } else
    {
        counters[cix] -= 1;
    };
    Book1[index] = (Book1[index] ^ (uint64_t)q);
    Book2[index] = (Book2[index] + (uint64_t)p);
    o.payload[0] = q;
    o.payload[1] = p;
    return (Book1[index] + (Book2[index] << 1)) ^ (uint64_t)(counters[cix] & 0xffff);
};

struct Processor {
    virtual ~Processor() = default;
    virtual uint64_t process(Order& o) = 0;
};

struct StrategyA_V final : Processor {
    uint64_t process(Order& o) override { return workA(o); }
};

struct StrategyB_V final : Processor {
    uint64_t process(Order& o) override { return workB(o); }
};

struct StrategyA_NV
{
    uint64_t run(Order& o) { return workA(o); }
};
struct StrategyB_NV
{
    uint64_t run(Order& o) { return workB(o); }
};


enum class Impl : uint8_t { Virtual=0, NonVirtual=1 };

struct Result {
    const char* pattern;  // fill at call site
    const char* impl;     // fill at call site
    int repeat;           // fill at call site
    uint64_t orders;
    uint64_t elapsed_ns;
    double   ops_per_sec;
    uint64_t checksum;
};

Result run_once(Impl impl,
                const char* pattern_name,
                const std::vector<Order>& orders_base,
                const std::vector<Strat>& assignments)
{
    reset_state();
    auto orders = orders_base; // identical inputs each run

    auto t0 = std::chrono::high_resolution_clock::now();

    uint64_t sum = 0;
    if (impl == Impl::Virtual) {
        StrategyA_V av; StrategyB_V bv; Processor* procs[2] = { &av, &bv };
        for (size_t i = 0; i < orders.size(); ++i) {
            int idx = (assignments[i] == Strat::A) ? 0 : 1;
            sum += procs[idx]->process(orders[i]);
        }
    } else {
        StrategyA_NV an; StrategyB_NV bn;
        for (size_t i = 0; i < orders.size(); ++i) {
            if (assignments[i] == Strat::A) sum += an.run(orders[i]);
            else                             sum += bn.run(orders[i]);
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    uint64_t ns = (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    double ops_s = (ns == 0) ? 0.0 : (double)orders.size() * 1e9 / (double)ns;

    g_sink += sum; // prevent optimization

    Result r{};
    r.pattern    = pattern_name;
    r.impl       = (impl == Impl::Virtual) ? "virtual" : "nonvirtual";
    r.repeat     = -1; // caller sets
    r.orders     = (uint64_t)orders.size();
    r.elapsed_ns = ns;
    r.ops_per_sec= ops_s;
    r.checksum   = sum;
    return r;
}

void write_csv_row(FILE* out, const Result& r) {
    std::fprintf(out, "%s,%s,%d,%llu,%llu,%.6f,%llu\n",
        r.pattern, r.impl, r.repeat,
        (unsigned long long)r.orders,
        (unsigned long long)r.elapsed_ns,
        r.ops_per_sec,
        (unsigned long long)r.checksum);
}

void warmup() {
    const size_t W = 1000000; // at least a million
    auto warm_orders = make_orders(W, 0xABCDEFULL);
    auto warm_asg    = make_assignments_mixed50(W, 0xBEEFULL);
    (void)run_once(Impl::Virtual,    "warm", warm_orders, warm_asg);
    (void)run_once(Impl::NonVirtual, "warm", warm_orders, warm_asg);
}
int main() {
    uint64_t ORDERS = ORDERS_DEFAULT;
    int REPEATS = REPEATS_DEFAULT;

    auto orders_base = make_orders(ORDERS, 0xDEADBEEFCAFEBABEULL);
    auto asg_homo  = make_assignments_homogeneous(ORDERS);
    auto asg_mixed = make_assignments_mixed50(ORDERS, 0x2222ULL);
    auto asg_burst = make_assignments_bursty(ORDERS);

    warmup();

    write_csv_header(stdout);
    std::ofstream file("results.csv");
    file << "pattern,impl,repeat,orders,elapsed_ns,ops_per_sec,checksum\n";

    auto emit = [&](Result r, int rep){
        r.repeat = rep;
        write_csv_row(stdout, r);
        file << r.pattern << "," << r.impl << "," << r.repeat << ","
             << r.orders << "," << r.elapsed_ns << ","
             << r.ops_per_sec << "," << r.checksum << "\n";
    };

    struct P { const char* name; const std::vector<Strat>* asg; } patterns[] = {
        {"homogeneous", &asg_homo},
        {"mixed50",     &asg_mixed},
        {"bursty64_16", &asg_burst}
    };

    for (auto& p : patterns) {
        for (Impl impl : { Impl::Virtual, Impl::NonVirtual }) {
            std::vector<uint64_t> times;
            times.reserve(REPEATS);

            for (int rep = 0; rep < REPEATS; ++rep) {
                Result r = run_once(impl, p.name, orders_base, *p.asg);
                emit(r, rep);
                times.push_back(r.elapsed_ns);
            }

            // compute best + median
            std::sort(times.begin(), times.end());
            uint64_t best = times.front();
            uint64_t median = times[times.size()/2];
            std::fprintf(stdout,
                "# summary pattern=%s impl=%s best_ns=%llu median_ns=%llu\n",
                p.name,
                (impl==Impl::Virtual)?"virtual":"nonvirtual",
                (unsigned long long)best,
                (unsigned long long)median);
        }
    }

    std::fprintf(stdout, "checksum_sink=%llu\n", (unsigned long long)g_sink);
    return 0;
}