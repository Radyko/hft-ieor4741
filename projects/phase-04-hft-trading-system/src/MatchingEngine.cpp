#include "../include/MatchingEngine.hpp"
#include "../include/OrderBook.hpp"

#include <algorithm>

template <typename PriceType, typename OrderIdType>
MatchingEngine<PriceType, OrderIdType>::MatchingEngine(TradeHandler on_trade)
    : on_trade_(std::move(on_trade)) {}

template <typename PriceType, typename OrderIdType>
void MatchingEngine<PriceType, OrderIdType>::set_trade_handler(TradeHandler cb) {
    on_trade_ = std::move(cb);
}

template <typename PriceType, typename OrderIdType>
bool MatchingEngine<PriceType, OrderIdType>::match_top(
    OrderBook<PriceType, OrderIdType>& book,
    OrderManager<PriceType, OrderIdType>& oms)
{
    const OrderT* bid = book.best_bid();
    const OrderT* ask = book.best_ask();
    if (!bid || !ask) return false;

    if (bid->price < ask->price) return false;

    auto bid_ptr = oms.get(bid->id);
    auto ask_ptr = oms.get(ask->id);
    if (!bid_ptr || !ask_ptr) return false;

    const int traded_qty = std::min(bid_ptr->remaining, ask_ptr->remaining);
    if (traded_qty <= 0) return false;

    const PriceType trade_px = ask->price;

    oms.apply_fill(bid_ptr->id, traded_qty);
    oms.apply_fill(ask_ptr->id, traded_qty);

    if (auto b = oms.get(bid_ptr->id);  b && b->remaining == 0) {
        book.erase_by_id(b->id);
    }
    if (auto a = oms.get(ask_ptr->id);  a && a->remaining == 0) {
        book.erase_by_id(a->id);
    }

    if (on_trade_) {
        TradeT t;
        t.buy_id   = bid->is_buy ? bid->id : ask->id;
        t.sell_id  = bid->is_buy ? ask->id : bid->id;
        t.price    = trade_px;
        t.quantity = traded_qty;
        t.timestamp = std::chrono::high_resolution_clock::now();
        on_trade_(t);
    }

    return true;
}

template <typename PriceType, typename OrderIdType>
void MatchingEngine<PriceType, OrderIdType>::match_while_crossed(
    OrderBook<PriceType, OrderIdType>& book,
    OrderManager<PriceType, OrderIdType>& oms)
{
    while (match_top(book, oms)) {
    }
}

template class MatchingEngine<double, int>;
