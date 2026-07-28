#pragma once
#include "messages.h"
#include <array>
#include <ostream>
#include <iomanip>

namespace itchbook {

struct EventStats {
    std::size_t trailing = 0;
    std::size_t bad_length = 0;
    std::size_t unknown = 0;
    std::array<uint64_t, 256> type_counts;
};

std::ostream& operator<<(std::ostream& os, const EventStats& s) {
    os << "trailing:   " << s.trailing   << '\n'
       << "bad_length: " << s.bad_length << '\n'
       << "unknown:    " << s.unknown    << '\n'
       << "\nby type:\n";
    
    uint64_t total_msg = 0;

    for (std::size_t t = 0; t < 256; ++t) {
        if (s.type_counts[t] == 0) continue;
        const bool printable = t >= 0x20 && t < 0x7f;
        os << "  " << (printable ? static_cast<char>(t) : '?')
           << " (0x" << std::hex << std::setw(2) << std::setfill('0') << t
           << std::dec << std::setfill(' ') << ")  "
           << std::setw(12) << s.type_counts[t] << '\n';
        total_msg += s.type_counts[t];
    }

    os << "\ntotal msg: " << total_msg << "\n";
    return os;
}

struct ParseState {
    std::array<Symbol, 65536> symbols;
};

}
