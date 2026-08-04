#pragma once
#include <iostream>
#include "session.h"

namespace itchbook {

inline void handle_system_event(const std::byte* m, Session& session) {
    const char code = static_cast<char>(m[11]);

    switch (code) {
        case 'O': 
            session.phase = MarketPhase::StartOfMessages;  
            break;

        case 'S': 
            session.phase = MarketPhase::SystemHours;      
            break;

        case 'Q': 
            session.phase = MarketPhase::RegularTrading;   
            break;

        case 'M': 
            session.phase = MarketPhase::AfterHours;       
            break;
        
        case 'E': 
            session.phase = MarketPhase::EndOfSystemHours; 
            break;
        
        case 'C': 
            session.phase = MarketPhase::EndOfMessages;    
            break;
    }
}

inline void handle_stock_directory(const std::byte* m, Session& session) {
    uint16_t locate = load_be<uint16_t>(m+1);
    Symbol& s = session.symbols[locate];
    std::memcpy(s.name, m+11, 8); 
    s.round_lot = load_be<uint32_t>(m+21);
    s.market_category = static_cast<char>(m[19]);
    s.financial_status = static_cast<char>(m[20]);
    s.is_etp = static_cast<char>(m[33]);
}

}
