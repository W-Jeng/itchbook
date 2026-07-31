#pragma once
#include "types.h"
#include "parse_state.h"
#include <stdexcept>

namespace itchbook {

inline void parse_add_order(const std::byte* m, ParseState& state) {
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

inline void parse_order_executed(const std::byte* m, ParseState& state) {
    // E
}

inline void parse_order_executed_with_price(const std::byte* m, ParseState& state) {
    // C
}

inline void parse_order_cancel(const std::byte* m, ParseState& state) {
    // X
}

inline void parse_order_delete(const std::byte* m, ParseState& state) {
    // D
}

inline void parse_order_replace(const std::byte* m, ParseState& state) {
    // U
}

}
