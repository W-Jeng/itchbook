#pragma once
#include "types.h"
#include "session.h"
#include <stdexcept>

namespace itchbook {

inline void handle_add_order(const std::byte* m, Session& state) {
    // A or F
    const StockLocate locate = load_be<uint16_t>(m+1);
    const OrderRefNum order_ref = load_be<uint64_t>(m+11);

    OrderRecord* o = state.order_store.emplace(locate, order_ref);

    if (!o)
        throw std::runtime_error("Unable to allocate for order");

    o->shares = load_be<uint32_t>(m+20);
    o->price = load_be<uint32_t>(m+32);
    o->locate = locate;
    o->side = static_cast<char>(m[19]);
}

inline void handle_order_executed(const std::byte* m, Session& state) {
    // E
}

inline void handle_order_executed_with_price(const std::byte* m, Session& state) {
    // C
}

inline void handle_order_cancel(const std::byte* m, Session& state) {
    // X
}

inline void handle_order_delete(const std::byte* m, Session& state) {
    // D
}

inline void handle_order_replace(const std::byte* m, Session& state) {
    // U
}

}
