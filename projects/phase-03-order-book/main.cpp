#include "market_snapshot.h"
#include "order_manager.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

enum class EventType { Bid, Ask, Execution };

struct Event {
    EventType type;
    double price = 0.0;
    int qty = 0;
    int id = -1;
};

std::vector<Event> load_feed(const std::string& filename)
{
    std::vector<Event> evs;
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "Could not open feed file: " << filename << "\n";
        return evs;
    }

    std::string kind;
    while (in >> kind) {
        if (kind[0] == '#') { // comment line starts with '#'
            std::string discard;
            std::getline(in, discard);
            continue;
        }

        if (kind == "BID") {
            double px; int q;
            if (in >> px >> q) {
                evs.push_back(Event{EventType::Bid, px, q, -1});
            }
        } else if (kind == "ASK") {
            double px; int q;
            if (in >> px >> q) {
                evs.push_back(Event{EventType::Ask, px, q, -1});
            }
        } else if (kind == "EXECUTION") {
            int order_id, q;
            if (in >> order_id >> q) {
                evs.push_back(Event{EventType::Execution, 0.0, q, order_id});
            }
        } else {
            std::string discard;
            std::getline(in, discard);
        }
    }
    return evs;
}

bool should_trade(const MarketSnapshot& snapshot)
{
    auto bid = snapshot.get_best_bid();
    auto ask = snapshot.get_best_ask();
    if (!bid || !ask) return false;
    return (ask->price - bid->price) < 0.05;
}

int main()
{
    MarketSnapshot snapshot;
    OrderManager om;

    const std::string feed_path = "sample_feed.txt";

    for (const auto& ev : load_feed(feed_path)) {
        switch (ev.type) {
            case EventType::Bid:
                // Snapshot semantics: qty==0 removes the level; else set to absolute qty
                snapshot.update_bid(ev.price, ev.qty);
                break;

            case EventType::Ask:
                snapshot.update_ask(ev.price, ev.qty);
                break;

            case EventType::Execution:
                if (ev.id != -1 && ev.qty > 0) {
                    om.handle_fill(ev.id, ev.qty);  // incremental fill
                }
                break;
        }

        if (should_trade(snapshot)) {
            auto bestBid = snapshot.get_best_bid();
            if (bestBid) {
                int id = om.place_order(Side::Buy, bestBid->price, 10);
                std::cout << "Placed BUY order at " << bestBid->price
                          << " (id=" << id << ")\n";
            }
        }
    }

    std::cout << "\nFinal active orders:\n";
    om.print_active_orders();
}
