#pragma once
#include "profiler.h"

namespace itchbook {

// ITCH Specifics
using StockLocate = uint16_t;
using OrderRefNum = uint64_t;

// Implementation Specific
using SlotIndex = uint32_t;

struct OrderRecord {
    uint32_t shares;
    uint32_t price;
    StockLocate locate;
    char side;
};


enum class Site : uint8_t {
    Dispatch, 
    AddOrder,
    Delete,
    Execute,
    Replace,
    COUNT
};

using Prof = Profiler<Site, ProfilingEnabled, 1'000'000>;

}
