#pragma once
#include "types.h"
#include "session.h"
#include "byte_order.h"
#include <stdexcept>
#include <iostream>
#include <array>
#include <unordered_set>

namespace itchbook {

inline void handle_noii(const std::byte* m, Session& session) {
    // I
    const char cross_type = static_cast<char>(m[48]);

    if (cross_type != 'O')
        return;

    const StockLocate locate = load_be<uint16_t>(m+1);
    uint32_t ref_price = load_be<uint32_t>(m+44);

    if (ref_price != 0) {
        auto& book = (*session.books)[locate];
        book.anchor(ref_price);
    }
}

}