#pragma once
#include "types.h"
#include "session.h"
#include "byte_order.h"
#include <stdexcept>
#include <iostream>
#include <array>
#include <unordered_set>

namespace itchbook {

inline void handle_trade(const std::byte* m, Session& session) {
    // P
    const StockLocate locate = load_be<uint16_t>(m + 1);
    auto& book = (*session.books)[locate];
    const uint32_t price = load_be<uint32_t>(m + 32);

    if (price > 0)
        book.anchor(price);
}

inline void handle_cross_trade(const std::byte* m, Session& session) {
    // Q
    const StockLocate locate = load_be<uint16_t>(m + 1);
    auto& book = (*session.books)[locate];
    const uint32_t price = load_be<uint32_t>(m + 27);

    if (price > 0)
        book.anchor(price);
}



}