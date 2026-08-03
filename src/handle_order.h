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
}

inline void handle_order_executed(const std::byte* m, Session& session) {
    // E
    const StockLocate locate = load_be<uint16_t>(m+1);
    const OrderRefNum order_ref = load_be<uint64_t>(m+11);

    OrderRecord* o = session.order_store.find(locate, order_ref);

    if (!o)
        throw std::runtime_error("Unable to find the order record in handle_order_executed");

    const uint32_t executed_shares = load_be<uint32_t>(m+19);
    const uint32_t taken = std::min(executed_shares, o->shares);
    o->shares -= taken;
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
    const char side = o->side;
    session.order_store.erase(locate, old_order_ref);
    const OrderRefNum new_order_ref = load_be<uint64_t>(m+19);
    OrderRecord* new_o = session.order_store.emplace(locate, new_order_ref);

    if (!new_o)
        throw std::runtime_error("Unable to allocate for order in handle_order_replace");

    new_o->shares = load_be<uint32_t>(m+27);
    new_o->price = load_be<uint32_t>(m+31);
    new_o->locate = locate;
    new_o->side = side;
    (*session.books)[locate].add(new_o->side, new_o->price, new_o->shares);
}

}
