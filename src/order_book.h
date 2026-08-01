#pragma once
#include <boost/container/flat_map.hpp>
#include <climits>
#include <vector>
#include <limits>
#include <algorithm>

namespace itchbook {

struct PriceLevel {
    uint32_t total_shares{0};
    uint32_t order_count{0};
};

class OrderBook {
public:
    explicit OrderBook(uint32_t num_levels = 4096)
        : m_num_levels(num_levels)
    {
        m_bids.levels.resize(num_levels);
        m_asks.levels.resize(num_levels);
    }

    struct Ladder {
        std::vector<PriceLevel> levels;
        uint32_t anchor_price = 0;
        uint32_t best_idx = 0;
        uint32_t tick = 100;
        uint32_t count = 0;
        bool anchored = false;

        uint32_t idx_of(uint32_t price) const {
            return (price-anchor_price) / tick;
        }

        uint32_t price_of(uint32_t idx) const {
            return anchor_price + idx * tick;
        }

        void init(uint32_t first_price, uint32_t num_levels) {
            tick = (first_price >= 10000) ? 100 : 1; // nasdaq specs
            const uint32_t half = num_levels / 2;
            anchor_price = (first_price > half * tick) ? (first_price - half * tick) : 0;
            anchored = true; 
        }
    };

    void add(char side, uint32_t price, uint32_t shares) {
        if (side == 'B')
            add_impl<true>(m_bids, price, shares);
        else
            add_impl<false>(m_asks, price, shares);
    }

    uint32_t best_bid() const {
        const uint32_t ladder_best = m_bids.count > 0 ? m_bids.price_of(m_bids.best_idx) : 0;
        const uint32_t overflow_best = m_bids_overflow.empty() ? 0 : m_bids_overflow.begin()->first;
        return std::max(ladder_best, overflow_best);
    }

    uint32_t best_ask() const {
        const uint32_t ladder_best = m_asks.count > 0 ? m_asks.price_of(m_asks.best_idx) : 0;
        const uint32_t overflow_best = m_asks_overflow.empty() ? 0 : m_asks_overflow.begin()->first;

        if (ladder_best == 0)
            return overflow_best;

        if (overflow_best == 0)
            return ladder_best;

        return std::max(ladder_best, overflow_best);
    }

private:
    Ladder m_bids;
    Ladder m_asks;
    boost::container::flat_map<uint32_t, PriceLevel, std::greater<>> m_bids_overflow;
    boost::container::flat_map<uint32_t, PriceLevel, std::less<>> m_asks_overflow;
    uint32_t m_num_levels;
    static constexpr uint32_t not_in_ladder = std::numeric_limits<uint32_t>::max();

    uint32_t ladder_idx(const Ladder& l, uint32_t price) const {
        if (price < l.anchor_price) 
            return not_in_ladder;

        const uint32_t i = l.idx_of(price);

        if (i >= l.levels.size())     
            return not_in_ladder;

        if ((price - l.anchor_price) % l.tick != 0) 
            return not_in_ladder;

        return i;
    }

    template <bool IsBid>
    void add_impl(Ladder& l, uint32_t price, uint32_t shares) {
        if (!l.anchored)
            l.init(price, m_num_levels);
        
        const uint32_t idx = ladder_idx(l, price);

        if (idx == not_in_ladder) {
            add_overflow<IsBid>(price, shares);
            return;
        }
        
        PriceLevel& p = l.levels[idx];
        p.total_shares += shares;
        const bool was_empty = (p.order_count == 0);
        p.order_count += 1;
        
        if (was_empty) {
            ++l.count;
            
            if (l.count == 1) {
                l.best_idx = idx;
            } else {
                if constexpr (IsBid) {
                    l.best_idx = std::max(l.best_idx, idx);
                } else {
                    l.best_idx = std::min(l.best_idx, idx);
                }
            }
        }
    }

    template <bool IsBid>
    void add_overflow(uint32_t price, uint32_t shares) {
        if constexpr (IsBid) {
            PriceLevel& p = m_bids_overflow[price];
            p.total_shares += shares;
            p.order_count += 1;
        } else {
            PriceLevel& p = m_asks_overflow[price];
            p.total_shares += shares;
            p.order_count += 1;
        }
    }
};

}