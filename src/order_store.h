#pragma once
#include "types.h"
#include "pool_allocator.h"
#include <unordered_map>
#include <array>

namespace itchbook {

struct NoShard {
    static constexpr std::size_t count = 1;
    static constexpr std::size_t of(StockLocate) { return 0; }
};


template <std::size_t Bits>
struct MaskShard {
    static constexpr std::size_t count = std::size_t{1} << Bits;
    static constexpr std::size_t of(StockLocate locate) { return locate & (count-1); }
};


template <typename Shard = NoShard>
class OrderStore {
public:
    explicit OrderStore(std::size_t capacity_per_shard) {
        for (auto& b: m_buckets)
            b.order_pool.init(capacity_per_shard);
    }
    
    OrderRecord* emplace(StockLocate locate, OrderRefNum ref) {
        Bucket& b = m_buckets[Shard::of(locate)];
        const SlotIndex slot_index = b.order_pool.alloc();

        if (slot_index == OrderPool::nil) {
            ++m_exhausted;
            return nullptr;
        }
            
        b.ref_to_slot[ref] = slot_index;
        return &b.order_pool[slot_index];
    }

    OrderRecord* find(StockLocate locate, OrderRefNum ref) {
        Bucket& b = m_buckets[Shard::of(locate)];
        const auto it = b.ref_to_slot.find(ref);

        if (it == b.ref_to_slot.end())
            return nullptr;

        const SlotIndex slot_index = it->second;
        return &b.order_pool[slot_index];
    }

    bool erase(StockLocate locate, OrderRefNum ref) {
        return false;
    };

    std::size_t exhausted() const {
        return m_exhausted;
    }


private:
    using OrderPool = PoolAllocator<OrderRecord>;

    struct Bucket {
        std::unordered_map<OrderRefNum, SlotIndex> ref_to_slot;
        OrderPool order_pool;
    };

    std::array<Bucket, Shard::count> m_buckets;
    std::size_t m_exhausted{0};

};

}
