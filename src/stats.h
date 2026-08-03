#pragma once
#include <ostream>
#include <iomanip>


namespace itchbook {

struct EventStats {
    std::size_t trailing = 0;
    std::size_t bad_length = 0;
    std::size_t unknown = 0;
    std::size_t total_msg_count = 0;
    std::array<uint64_t, 256> type_counts{};
};

inline std::ostream& operator<<(std::ostream& os, const EventStats& s) {
    os << "trailing:   " << s.trailing   << '\n'
       << "bad_length: " << s.bad_length << '\n'
       << "unknown:    " << s.unknown    << '\n'
       << "\nby type:\n";
    
    uint64_t total_msg = 0;

    for (std::size_t t = 0; t < 256; ++t) {
        if (s.type_counts[t] == 0) 
            continue;

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


struct ValidationStats {
    uint64_t crossed_count = 0;
    uint64_t execution_volume = 0;
    uint64_t trade_volume = 0;
    uint64_t cross_volume = 0;
};


inline std::ostream& operator<<(std::ostream& os, const ValidationStats& v) {
    os << "crossed during regular hours: " << v.crossed_count << "\n"
       << "execution volume (E+C):      " << v.execution_volume << "\n"
       << "trade volume (P):            " << v.trade_volume << "\n"
       << "cross volume (Q):            " << v.cross_volume << "\n"
       << "total volume:                "
       << (v.execution_volume + v.trade_volume + v.cross_volume) << "\n";
    return os;
}

}
