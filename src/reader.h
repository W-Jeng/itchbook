#pragma once
#include "byte_order.h"
#include "mapped_file.h"
#include "messages.h"
#include "parse_state.h"
#include "parse_admin_msg.h"
#include "parse_order_msg.h"
#include "order_store.h"
#include <array>
#include <cstdint>
#include <iostream>

namespace itchbook {

class Reader {
public:
    void run(const MappedFile& f) {
        const std::byte* const begin = f.data();
        const std::byte* p = begin;
        const std::byte* const end = begin + f.size();
        
        while (end - p >= 2) {
            const uint16_t len = msg_len(p);

            if (static_cast<std::size_t>(end-p) < 2u + len) 
                break;
            
            dispatch(p+2, len);
            p += 2 + len;
        }

        m_stats.trailing = static_cast<std::size_t>(end-p);
        // print_symbols(std:cout, m_state.symbols);
    }

    const ParseState& state() const { return m_state; }
    const EventStats& stats() const { return m_stats; }

private:
    void dispatch(const std::byte* p, uint16_t len) {
        const uint8_t t = std::to_integer<uint8_t>(p[0]);
        const uint8_t expect = expected_len[t];
        ++m_stats.type_counts[t];

        if (expect == 0) {
            ++m_stats.unknown;
            return;
        }

        if (expect != len) {
            ++m_stats.bad_length;
            return;
        }   

        switch (t) {
            case 'S':
                parse_system_event(p, m_state);
                break;

            case 'R':
                parse_stock_directory(p, m_state);
                break;
            
            case 'A':
                parse_add_order(p, m_state);
                break;

            case 'F':
                parse_add_order(p, m_state);
                break;

            case 'E':
                parse_order_executed(p, m_state);
                break;
            
            case 'C':
                parse_order_executed_with_price(p, m_state);
                break;
            
            case 'X':
                parse_order_cancel(p, m_state);
                break;
            
            case 'D':
                parse_order_delete(p, m_state);
                break;
            
            case 'U':
                parse_order_replace(p, m_state);
                break;
            
            default:
                break;
        }
    }

    [[gnu::always_inline]]
    static uint16_t msg_len(const std::byte* p) {
        return (
            (uint16_t(std::to_integer<uint8_t>(p[0])) << 8)
            | std::to_integer<uint8_t>(p[1])
        );
    }
    
    ParseState m_state{};
    EventStats m_stats{};
};

}
