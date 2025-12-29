#include "types.h"
#include "market_data_client.h"
#include "arbitrage_engine.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "Cross-Market Arbitrage Platform" << std::endl;
    
    Config config;
    std::cout << "Config loaded. Min profit: " << config.min_profit_threshold << std::endl;
    
    PolymarketClient client;
    std::cout << "Polymarket client created" << std::endl;
    
    ArbitrageEngine engine(&config);
    std::cout << "Arbitrage engine initialized" << std::endl;
    
    MarketData data;
    data.market_id = "test_001";
    data.market = MARKET_POLYMARKET;
    data.event_name = "Test Event";
    data.best_bid = 0.45;
    data.best_ask = 0.55;
    data.bid_size = 1000.0;
    data.ask_size = 1000.0;
    data.is_valid = true;
    
    std::cout << "Market data: " << data.event_name << " bid=" << data.best_bid 
              << " ask=" << data.best_ask << std::endl;
    
    return 0;
}
