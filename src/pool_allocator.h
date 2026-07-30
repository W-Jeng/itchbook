#pragma once
#include <vector>
#include <type_traits>
#include <cstdint>
#include <limits>

namespace itchbook {

template<typename T, typename Index = uint32_t>
class PoolAllocator {
    static_assert(std::is_trivially_copyable_v<T>);
    static_assert(sizeof(T) >= sizeof(Index));
public:
    using index_type = Index;
    static constexpr nil = std::numeric_limits<Index>::max();

    explicit PoolAllocator(std::size_t cap): m_slots(cap) {}

    Index alloc() {
        if (m_free != nil) {
            const Index i = m_free;
            m_free = m_slots[i].next;
            return i;
        }

        if (m_high < m_slots.size())
            return m_high++;

        ++m_exhausted;
        return nil;
    }

    void release(Index i) {
        m_slots[i].next = m_free;
        m_free = i;
    }

    T& operator[](Index i) {
        return m_slots[i].value;
    }

    const T& operator[](Index i) const {
        return m_slots[i].value;
    }

    std::size_t capacity() const { return m_slots.size(); }
    std::size_t high_water() const { return m_high; }
    std::size_t exhausted() const { return m_exhausted; }

private:
    union Slot {
        T value;
        Index next;
        Slot() : next(nil) {}
    };

    std::vector<Slot> m_slots;
    Index m_free = nil;
    Index m_high = 0;
    std::size_t m_exhausted = 0;
};
 

struct NoShard {
    static constexpr std::size_t count = 1;
    static constexpr std::size_t of(uint16_t) { return 0; }
};


template<std::size_t Bits>
struct MaskShard {
    static constexpr std::size_t count = std::size_t{1} << Bits;
    static constexpr std::size_t of(uint16_t locate) { return locate & (count-1); }
};


// template<typename T, typename Shard = NoShard>
// class 

}
