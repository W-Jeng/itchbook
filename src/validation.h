#pragma once
#include "session.h"
#include "byte_order.h"
#include "types.h"
#include "fmt/format.h"

namespace itchbook {

inline void validate_book(const std::byte* p, Session& session) {
    const uint8_t t = std::to_integer<uint8_t>(p[0]);

    switch (t) {
        case 'E': {
            const uint32_t shares = load_be<uint32_t>(p + 19);
            session.validation.execution_volume += shares;
            break;
        }
        case 'C': {
            const uint32_t shares = load_be<uint32_t>(p + 19);
            session.validation.execution_volume += shares;
            break;
        }
        case 'P': {
            const uint32_t shares = load_be<uint32_t>(p + 20);
            session.validation.trade_volume += shares;
            break;
        }
        case 'Q': {
            const uint64_t shares = load_be<uint64_t>(p + 11);
            session.validation.cross_volume += shares;
            break;
        }
        default:
            break;
    }

    // crossed-book check
    if (session.phase != MarketPhase::RegularTrading)
        return;

    if (!(t == 'A' || t == 'F' || t == 'D' || t == 'E' ||
          t == 'C' || t == 'X' || t == 'U'))
        return;

    const StockLocate locate = load_be<uint16_t>(p + 1);
    const auto& book = (*session.books)[locate];
    const uint32_t bid = book.best_bid();
    const uint32_t ask = book.best_ask();

    if (bid > 0 && ask > 0 && bid >= ask)
        ++session.validation.crossed_count;
}

}
