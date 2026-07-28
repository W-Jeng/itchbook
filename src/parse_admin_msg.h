#pragma once
#include <iostream>

namespace itchbook {

[[gnu::always_inline]]
inline void parse_system_event(const std::byte* m, ParseState& state) {
    // do nothing
}

[[gnu::always_inline]]
inline void parse_stock_directory(const std::byte* m, ParseState& state) {
    uint16_t locate = load_be<uint16_t>(m+1);
    Symbol& s = state.symbols[locate];
    std::memcpy(s.name, m+11, 8); 
    s.round_lot = load_be<uint32_t>(m+21);
    s.market_category = static_cast<char>(m[19]);
    s.financial_status = static_cast<char>(m[20]);
    s.is_etp = static_cast<char>(m[33]);
}

};
