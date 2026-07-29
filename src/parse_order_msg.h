#pragma once

namespace itchbook {

inline void parse_add_order(const std::byte* m, ParseState& state) {
    // A or F
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
