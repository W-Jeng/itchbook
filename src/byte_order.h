#pragma once
#include <cstdint>
#include <cstddef>

namespace itchbook {
    
template<typename T>
T load_be(const std::byte* p) {
    T v;
    std::memcpy(&v, p, sizeof(T));

    if constexpr (std::endian::native == std::endian::little)
        v  = std::byteswap(v);

    return v;
}

inline uint64_t load_be48(const std::byte* p) {
    uint64_t v = 0;

    for (int i = 0; i < 6; ++i) 
        v = (v << 8) | std::to_integer<uint8_t>(p[i]);

    return v;
}

}
