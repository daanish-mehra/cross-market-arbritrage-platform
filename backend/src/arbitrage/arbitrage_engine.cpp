#include "arbitrage_engine.h"
#include "types.h"
#include <map>
#include <string>

ArbitrageEngine::ArbitrageEngine(Config* config) {
    this->config = config;
    this->opportunity_callback = NULL;
    this->market_data_map = NULL;
}

ArbitrageEngine::~ArbitrageEngine() {
}

void ArbitrageEngine::update_market_data(MarketData* data) {
}

void ArbitrageEngine::set_opportunity_function(void (*func)(ArbitrageOpportunity*)) {
    opportunity_callback = func;
}

void ArbitrageEngine::check_for_opportunities() {
}

double ArbitrageEngine::compute_profit(MarketData* buy, MarketData* sell) {
    return 0.0;
}

double ArbitrageEngine::compute_max_size(MarketData* buy, MarketData* sell) {
    return 0.0;
}
