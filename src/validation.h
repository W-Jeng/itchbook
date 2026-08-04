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

        case 'H': {
            const StockLocate loc = load_be<uint16_t>(p + 1);
            const char state = static_cast<char>(p[19]);
            if (loc == 5142 || loc == 8615 || loc == 4208 || loc == 7598)
                fmt::print("HALT locate={} state={} \n", loc, state);
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

    if (bid > 0 && ask > 0 && bid >= ask) {
        ++session.validation.crossed_count;

        if (session.validation.crossed_count <= 8000) {
            const uint64_t ts = load_be48(p + 5);
            const double secs = ts / 1e9;
            const int h = int(secs) / 3600;
            const int m = (int(secs) % 3600) / 60;
            const int s = int(secs) % 60;
            const StockLocate locate = load_be<uint16_t>(p + 1);
            fmt::print("CROSSED {:02d}:{:02d}:{:02d} locate={} ({}) bid={} ask={}\n",
                h, m, s, locate, ticker(session.symbols[locate]), bid, ask);
        }
}
}

}
