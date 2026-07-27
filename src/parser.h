#pragma once
#include "byte_order.h"
#include "mapped_file.h"
#include "messages.h"
#include <array>

namespace itchbook {

class ITCHParser {
public:
    void run(const MappedFile& f) {

    }

private:
    void dispatch(const std::byte* m, uint16_t len) {

    }

    void on_add_order(const std::byte* m) {

    }

    void on_add_order(const std::byte* m) {

    }

    std::array<Symbol, 65536> m_symbols;
    std::array<uint64_t, 256> m_type_counts;
};

}
