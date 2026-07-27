#pragma once
#include <cstdint>

namespace itchbook {

struct Symbol {
    char        name[8];
    uint32_t    round_lot;
    char        market_category;
    char        financial_status;
    char        is_etp;
};

struct OrderRecord {
    uint32_t shares;
    uint32_t price;
    uint16_t locate;
    char side;
};

}