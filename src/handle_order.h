#pragma once
#include "types.h"
#include "session.h"
#include <stdexcept>
#include <iostream>
#include <array>
#include <unordered_set>

namespace itchbook {

inline void handle_add_order(const std::byte* m, Session& session) {
    // A or F
    const StockLocate locate = load_be<uint16_t>(m+1);
    const OrderRefNum order_ref = load_be<uint64_t>(m+11);

    OrderRecord* o = session.order_store.emplace(locate, order_ref);

    if (!o)
        throw std::runtime_error("Unable to allocate for order in handle_add_order");

    o->shares = load_be<uint32_t>(m+20);
    o->price = load_be<uint32_t>(m+32);
    o->locate = locate;
    o->side = static_cast<char>(m[19]);
    (*session.books)[locate].add(o->side, o->price, o->shares);

    // static std::array<uint8_t, 65536> logged{};
    // static std::unordered_set<uint16_t> printed;
    // auto& book = (*session.books)[locate];

    // if (book.num_overflow_accessed() == 1 && !printed.contains(locate) && ticker(session.symbols[locate]) =="MSFT") {
    //     const auto& ar_bid = book.anchored_bid_ranges();
    //     const auto& ar_ask = book.anchored_ask_ranges();
    //     fmt::print("overflow: locate={} ({}) price=${} side={}, lower_bid={}, upper_bid={}, lower_ask={}, upper_ask={}\n",
    //         locate, ticker(session.symbols[locate]), o->price, o->side, ar_bid.first, ar_bid.second, ar_ask.first, ar_ask.second);
    //     printed.insert(locate);
    // }

}

inline void handle_order_executed(const std::byte* m, Session& session) {
    // E
    const StockLocate locate = load_be<uint16_t>(m+1);
    const OrderRefNum order_ref = load_be<uint64_t>(m+11);

    OrderRecord* o = session.order_store.find(locate, order_ref);

    if (!o)
        throw std::runtime_error("Unable to find the order record in handle_order_executed");

    uint32_t executed_shares = load_be<uint32_t>(m+19);
    o->shares -= executed_shares;
    bool need_remove = (o->shares == 0);

    (*session.books)[locate].subtract(o->side, o->price, executed_shares, need_remove);

    if (need_remove)
        session.order_store.erase(locate, order_ref);
}

inline void handle_order_executed_with_price(const std::byte* m, Session& session) {
    // C
    handle_order_executed(m, session);
}

inline void handle_order_cancel(const std::byte* m, Session& session) {
    // X
    const StockLocate locate = load_be<uint16_t>(m+1);
    const OrderRefNum order_ref = load_be<uint64_t>(m+11);

    OrderRecord* o = session.order_store.find(locate, order_ref);

    if (!o)
        throw std::runtime_error("Unable to find the order record for handle_order_cancel!");

    uint32_t cancelled_shares = load_be<uint32_t>(m+19);
    o->shares -= cancelled_shares;
    bool need_remove = (o->shares == 0);
    (*session.books)[locate].subtract(o->side, o->price, cancelled_shares, need_remove);

    if (need_remove)
        session.order_store.erase(locate, order_ref);
}

inline void handle_order_delete(const std::byte* m, Session& session) {
    // D
    const StockLocate locate = load_be<uint16_t>(m+1);
    const OrderRefNum order_ref = load_be<uint64_t>(m+11);

    OrderRecord* o = session.order_store.find(locate, order_ref);

    if (!o)
        throw std::runtime_error("Unable to find the order record for handle_order_cancel!");

    (*session.books)[locate].subtract(o->side, o->price, o->shares, true);
    session.order_store.erase(locate, order_ref);
}

inline void handle_order_replace(const std::byte* m, Session& session) {
    // U
    const StockLocate locate = load_be<uint16_t>(m+1);
    const OrderRefNum old_order_ref = load_be<uint64_t>(m+11);

    OrderRecord* o = session.order_store.find(locate, old_order_ref);

    if (!o)
        throw std::runtime_error("Unable to find the order record for handle_order_replace!");

    (*session.books)[locate].subtract(o->side, o->price, o->shares, true);
    const OrderRefNum new_order_ref = load_be<uint64_t>(m+19);
    OrderRecord* new_o = session.order_store.emplace(locate, new_order_ref);

    if (!new_o)
        throw std::runtime_error("Unable to allocate for order in handle_order_replace");

    new_o->shares = load_be<uint32_t>(m+27);
    new_o->price = load_be<uint32_t>(m+31);
    new_o->locate = locate;
    new_o->side = o->side;
    (*session.books)[locate].add(new_o->side, new_o->price, new_o->shares);

    // erase old order_ref
    session.order_store.erase(locate, old_order_ref);
}

}
