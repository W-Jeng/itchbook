#pragma once
#include "messages.h"
#include "order_store.h"
#include "order_book.h"
#include "fmt/format.h"
#include <array>
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

struct Session {
    std::array<Symbol, 65536> symbols{};
    std::unique_ptr<std::array<OrderBook, 65536>> books = 
        std::make_unique<std::array<OrderBook, 65536>>();
    OrderStore<NoShard> order_store{2'000'000};
    Prof profiler;

    void summary_overflow_accessed() {
        // in add_overflow, log the first few per symbol
        std::array<uint32_t, 65536> overflow_by_locate{};

        for (uint32_t locate = 0; locate < (*books).size(); ++locate) {
            const auto& book = (*books)[locate];
            overflow_by_locate[locate] += book.num_overflow_accessed();
        }

        std::vector<std::pair<uint32_t, uint16_t>> sorted;

        for (uint32_t i = 0; i < 65536; ++i)
            if (overflow_by_locate[i]) sorted.push_back({overflow_by_locate[i], i});

        std::sort(sorted.rbegin(), sorted.rend());

        for (int i = 0; i < 5 && i < sorted.size(); ++i)
            fmt::print("locate {} ({}): {} overflows\n",
                sorted[i].second, ticker(symbols[sorted[i].second]), sorted[i].first);

    }
};

}
