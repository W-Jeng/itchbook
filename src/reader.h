#pragma once
#include "byte_order.h"
#include "mapped_file.h"
#include "messages.h"
#include "session.h"
#include "handle_admin.h"
#include "handle_order.h"
#include "handle_noii.h"
#include "handle_trade.h"
#include "order_store.h"
#include "validation.h"
#include "utils.h"
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
        // print_symbols(std:cout, m_session.symbols);
        m_session.summary_overflow_accessed();
        std::cout << "High water all: " << m_session.order_store.high_water_all() << "\n";
        report_latency(m_session.profiler);
        std::cout << m_session.validation;
    }

    const Session& state() const { return m_session; }
    const EventStats& stats() const { return m_stats; }

private:
    Session m_session{};
    EventStats m_stats{};

    void dispatch(const std::byte* p, uint16_t len) {
        const uint8_t t = std::to_integer<uint8_t>(p[0]);
        const uint8_t expect = expected_len[t];
        ++m_stats.type_counts[t];
        ++m_stats.total_msg_count;

        if (expect == 0) {
            ++m_stats.unknown;
            return;
        }

        if (expect != len) {
            ++m_stats.bad_length;
            return;
        }   

        const bool sample = ((m_stats.total_msg_count & SampleEveryN) == 0);
        const Site site = sample ? site_of(p, t) : Site::Other;

        if (sample)
            m_session.profiler.start(site);

        switch (t) {
            case 'S':
                handle_system_event(p, m_session);
                break;

            case 'R':
                handle_stock_directory(p, m_session);
                break;
            
            case 'A':
                handle_add_order(p, m_session);
                break;

            case 'F':
                handle_add_order(p, m_session);
                break;

            case 'E':
                handle_order_executed(p, m_session);
                break;
            
            case 'C':
                handle_order_executed_with_price(p, m_session);
                break;
            
            case 'X':
                handle_order_cancel(p, m_session);
                break;
            
            case 'D':
                handle_order_delete(p, m_session);
                break;
            
            case 'U':
                handle_order_replace(p, m_session);
                break;

            case 'I':
                handle_noii(p, m_session);
                break;
            
            case 'P':
                handle_trade(p, m_session);
                break;

            case 'Q':
                handle_cross_trade(p, m_session);
                break;

            default:
                break;
        }

        if (sample)
            m_session.profiler.stop(site);

        validate_book(p, m_session);
    }

    [[gnu::always_inline]]
    static uint16_t msg_len(const std::byte* p) {
        return (
            (uint16_t(std::to_integer<uint8_t>(p[0])) << 8)
            | std::to_integer<uint8_t>(p[1])
        );
    }
    
    Site site_of(const std::byte* p, uint8_t t) const {
        const StockLocate loc = load_be<uint16_t>(p + 1);
        const bool anchored = (*m_session.books)[loc].is_anchored();

        switch (t) {
            case 'A':
                return anchored ? Site::AddOrder : Site::AddOrderPre;

            case 'F':
                return anchored ? Site::AddOrder : Site::AddOrderPre;

            case 'D':
                return anchored ? Site::Delete : Site::DeletePre;

            case 'E':
                return anchored ? Site::Execute : Site::ExecutePre;

            case 'C':
                return anchored ? Site::Execute : Site::ExecutePre;

            case 'X':
                return anchored ? Site::Cancel : Site::CancelPre;

            case 'U':
                return anchored ? Site::Replace : Site::ReplacePre;

            default:
                return Site::Other;
        }
    }

};

}
