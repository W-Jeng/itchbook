#pragma once
#include <boost/container/flat_map.hpp>
#include "fmt/format.h"
#include <climits>
#include <vector>
#include <limits>
#include <algorithm>
#include <utility>

namespace itchbook {

struct PriceLevel {
    uint32_t total_shares{0};
    uint32_t order_count{0};
};

class OrderBook {
public:
    OrderBook(uint32_t num_levels = 4096)
        : m_num_levels(num_levels)
        , overflow_accessed(0)
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

    void subtract(char side, uint32_t price, uint32_t shares, bool order_removed) {
        if (side == 'B') 
            subtract_impl<true>(m_bids, price, shares, order_removed);
        else
            subtract_impl<false>(m_asks, price, shares, order_removed);
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

        return std::min(ladder_best, overflow_best);
    }
    
    bool is_anchored() const {
        return m_bids.anchored && m_asks.anchored;
    }

    uint32_t num_overflow_accessed() const {
        return overflow_accessed;
    }

    std::pair<uint32_t, uint32_t> anchored_bid_ranges() const {
        const auto& l = m_bids;
        return {l.anchor_price, l.anchor_price+l.levels.size()*l.tick};
    }

    std::pair<uint32_t, uint32_t> anchored_ask_ranges() const {
        const auto& l = m_asks;
        return {l.anchor_price, l.anchor_price+l.levels.size()*l.tick};
    }

    void anchor(uint32_t price) {
        if (m_bids.anchored && m_asks.anchored)
            return;

        m_bids.init(price, m_num_levels);
        move_to_ladder<true>(m_bids, m_bids_overflow);

        m_asks.init(price, m_num_levels);
        move_to_ladder<false>(m_asks, m_asks_overflow);

        // reset it 
        overflow_accessed = 0;
    }
    
private:
    Ladder m_bids;
    Ladder m_asks;
    boost::container::flat_map<uint32_t, PriceLevel, std::greater<>> m_bids_overflow;
    boost::container::flat_map<uint32_t, PriceLevel, std::less<>> m_asks_overflow;
    uint32_t m_num_levels;
    uint32_t overflow_accessed;
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
        if (!l.anchored) {
            add_overflow<IsBid>(price, shares);
            return;
        }

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

    template<bool IsBid>
    auto& overflow_map() {
        ++overflow_accessed;

        if constexpr (IsBid)
            return m_bids_overflow;
        else
            return m_asks_overflow;
    }

    template <bool IsBid>
    void add_overflow(uint32_t price, uint32_t shares) {
        auto& overflow = overflow_map<IsBid>();
        PriceLevel& p = overflow[price];
        p.total_shares += shares;
        p.order_count += 1;
    }

    template <bool IsBid>
    void subtract_impl(Ladder& l, uint32_t price, uint32_t shares, bool order_removed) {
        if (!l.anchored) {
            subtract_overflow<IsBid>(price, shares, order_removed);
            return;
        }

        const uint32_t idx = ladder_idx(l, price);

        if (idx == not_in_ladder) {
            subtract_overflow<IsBid>(price, shares, order_removed);
            return;
        }

        PriceLevel& p = l.levels[idx];
        p.total_shares -= shares;
        
        if (order_removed)
            p.order_count -= 1;

        if (p.order_count == 0 && l.count > 0) {
            l.count -= 1;

            if (l.count > 0 && idx == l.best_idx)
                scan_next_best<IsBid>(l);
        }
    }

    template<bool IsBid>
    void scan_next_best(Ladder& l) {
        if constexpr (IsBid) {
            while (l.levels[l.best_idx].order_count == 0) {
                if (l.best_idx == 0)
                    break;

                --l.best_idx;
            }
        } else {
            while (l.levels[l.best_idx].order_count == 0) {
                if (l.best_idx+1 >= l.levels.size())
                    break;

                ++l.best_idx;
            }
        }
    }

    template <bool IsBid>
    void subtract_overflow(uint32_t price, uint32_t shares, bool order_removed) {
        auto& overflow = overflow_map<IsBid>();
        auto it = overflow.find(price);

        if (it == overflow.end())
            throw std::runtime_error("Overflow price level is not found");

        PriceLevel& p = it->second;
        p.total_shares -= shares;

        if (order_removed) {
            p.order_count -= 1;

            if (p.order_count == 0)
                overflow.erase(it);
        }
    }

    template <bool IsBid>
    void move_to_ladder(Ladder& l, auto& overflow) {
        std::remove_reference_t<decltype(overflow)> remaining;

        for (auto& [price, p_level] : overflow) {
            const uint32_t idx = ladder_idx(l, price);

            if (idx == not_in_ladder) {
                remaining.emplace(price, p_level);
                continue;
            }

            l.levels[idx] = p_level;
            ++l.count;

            if (l.count == 1) {
                l.best_idx = idx;
            } else {
                if constexpr (IsBid)             
                    l.best_idx = std::max(l.best_idx, idx);
                else 
                    l.best_idx = std::min(l.best_idx, idx);
            }
        }

        overflow = std::move(remaining);
    }
};

}
