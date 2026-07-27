#pragma once
#include "profiling.h"

namespace itchbook {
    
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
