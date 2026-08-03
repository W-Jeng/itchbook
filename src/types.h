#pragma once
#include "profiler.h"
#include <string_view>

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
    AddOrder,
    AddOrderPre,
    Delete,
    DeletePre,
    Execute,
    ExecutePre,
    Replace,
    ReplacePre,
    Cancel,
    CancelPre,
    Other,
    OtherPre,
    COUNT
};


inline std::string_view site_to_str(Site site) {
    switch (site) {
        case Site::AddOrder:    return "AddOrder";
        case Site::AddOrderPre: return "AddOrder(pre)";
        case Site::Delete:      return "Delete";
        case Site::DeletePre:   return "Delete(pre)";
        case Site::Execute:     return "Execute";
        case Site::ExecutePre:  return "Execute(pre)";
        case Site::Replace:     return "Replace";
        case Site::ReplacePre:  return "Replace(pre)";
        case Site::Cancel:      return "Cancel";
        case Site::CancelPre:   return "Cancel(pre)";
        case Site::Other:       return "Other";
        case Site::COUNT:       return "COUNT";
    }
    return "Unknown";
}


enum class MarketPhase : uint8_t {
    PreOpen,
    StartOfMessages,
    SystemHours,
    RegularTrading,
    AfterHours,
    EndOfSystemHours,
    EndOfMessages
};


inline constexpr bool ProfilingEnabled = true;
using Prof = Profiler<Site, ProfilingEnabled, 1'000'000>;
inline constexpr uint8_t SampleEveryN = 0x3F;

}
