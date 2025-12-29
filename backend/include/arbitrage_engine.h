#pragma once

#include "types.h"

class ArbitrageEngine {
public:
    ArbitrageEngine(Config* config);
    ~ArbitrageEngine();
    
    void update_market_data(MarketData* data);
    void set_opportunity_function(void (*func)(ArbitrageOpportunity*));
    
private:
    void check_for_opportunities();
    double compute_profit(MarketData* buy, MarketData* sell);
    double compute_max_size(MarketData* buy, MarketData* sell);
    
    Config* config;
    void (*opportunity_callback)(ArbitrageOpportunity*);
    void* market_data_map;
};

