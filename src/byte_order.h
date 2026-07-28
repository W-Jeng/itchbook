#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <bit>

namespace itchbook {
    
template <typename T>
inline T load_be(const std::byte* p) {
    T v;
    std::memcpy(&v, p, sizeof(T));

    if constexpr (std::endian::native == std::endian::little) {
        if constexpr      (sizeof(T) == 2) v = __builtin_bswap16(v);
        else if constexpr (sizeof(T) == 4) v = __builtin_bswap32(v);
        else if constexpr (sizeof(T) == 8) v = __builtin_bswap64(v);
    }

    return v;
}

inline uint64_t load_be48(const std::byte* p) {
    uint64_t v = 0;

    for (int i = 0; i < 6; ++i) 
        v = (v << 8) | std::to_integer<uint8_t>(p[i]);

    return v;
}

}
