#pragma once
#include <cstdint>
#include <array>
#include <iostream>
#include <ostream>
#include <iomanip>

namespace itchbook {

struct Symbol {
    char        name[8];
    uint32_t    round_lot;
    char        market_category;
    char        financial_status;
    char        is_etp;
};

inline std::string_view ticker(const Symbol& s) {
    std::string_view v(s.name, 8);
    const auto n = v.find_last_not_of(' ');
    return (n == std::string_view::npos) ? std::string_view{} : v.substr(0, n+1);
}

inline bool is_populated(const Symbol& s) {
    return s.name[0] != '\0';
}

inline std::ostream& operator<<(std::ostream& os, const Symbol& s) {
    return os << std::left  << std::setw(9) << ticker(s)
              << std::right << std::setw(6) << s.round_lot
              << "  " << s.market_category
              << ' '  << s.financial_status
              << ' '  << s.is_etp;
}

inline void print_symbols(std::ostream& os, 
    const std::array<Symbol, 65536>& syms) {
    for (std::size_t i = 0; i < syms.size(); ++i) {
        if (!is_populated(syms[i]))
            continue;
        os << syms[i] << "\n";
    }
}

inline constexpr std::array<uint8_t, 256> expected_len = [] {
    std::array<uint8_t, 256> t{};
    t['S'] = 12; t['R'] = 39; t['H'] = 25; t['Y'] = 20; t['L'] = 26;
    t['V'] = 35; t['W'] = 12; t['K'] = 28; t['J'] = 35; t['h'] = 21;
    t['A'] = 36; t['F'] = 40; t['E'] = 31; t['C'] = 36; t['X'] = 23;
    t['D'] = 19; t['U'] = 35; t['P'] = 44; t['Q'] = 40; t['B'] = 19;
    t['I'] = 50; t['N'] = 20;
    return t;
}();


}
