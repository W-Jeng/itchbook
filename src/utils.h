#pragma once 
#include "profiler.h"
#include "types.h"

namespace itchbook {

void report_latency(auto& profiler) {
    for (std::size_t i = 0; i < static_cast<std::size_t>(Site::COUNT); ++i) {
        auto& buf = profiler.samples(static_cast<Site>(i));
        
        if (buf.size() == 0) 
            continue;
        
        std::string_view name = site_to_str(static_cast<Site>(i));
        fmt::print("{:>14}  n={:>8}  p50={:>6}  p99={:>6}  p99.9={:>6}  dropped={}\n",
            name, buf.size(),
            buf.pct(50), buf.pct(99), buf.pct(99.9), buf.dropped());
    }
}

}
