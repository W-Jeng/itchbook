#pragma once
#include "messages.h"
#include "order_store.h"
#include "order_book.h"
#include "stats.h"
#include "fmt/format.h"
#include <array>
#include <ostream>
#include <iomanip>
#include <vector>

namespace itchbook {

struct Session {
    std::array<Symbol, 65536> symbols{};
    std::unique_ptr<std::array<OrderBook, 65536>> books = 
        std::make_unique<std::array<OrderBook, 65536>>();
    OrderStore<NoShard> order_store{2'000'000};
    Prof profiler;
    MarketPhase phase = MarketPhase::PreOpen;
    ValidationStats validation{};
    std::array<char, 65536> halt_state{};

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
            fmt::print("Locate {} ({}): {} overflows\n",
                sorted[i].second, ticker(symbols[sorted[i].second]), sorted[i].first);
    }
};

}
