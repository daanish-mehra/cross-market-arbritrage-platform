#include "types.h"
#include "market_data_client.h"
#include "arbitrage_engine.h"
#include <iostream>
#include <signal.h>
#include <unistd.h>

bool should_run = true;
ArbitrageEngine* global_engine = NULL;

void handle_signal(int sig) {
    std::cout << "\nShutting down..." << std::endl;
    should_run = false;
}

void on_opportunity(ArbitrageOpportunity* opp) {
    double profit_pct = opp->profit_percentage * 100.0;
    std::cout << "Opportunity: " << opp->event_id << " - " << profit_pct 
              << "% profit, buy at " << opp->buy_price << " sell at " 
              << opp->sell_price << std::endl;
}

void on_market_update(MarketData* data) {
    if (global_engine != NULL) {
        global_engine->update_market_data(data);
    }
}

int main(int argc, char* argv[]) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    std::cout << "Cross-Market Arbitrage Platform" << std::endl;
    std::cout << "Starting..." << std::endl;
    
    Config config;
    
    ArbitrageEngine engine(&config);
    engine.set_opportunity_function(on_opportunity);
    global_engine = &engine;
    
    PolymarketClient polymarket;
    polymarket.set_update_function(on_market_update);
    
    std::cout << "Connecting to Polymarket..." << std::endl;
    if (!polymarket.connect()) {
        std::cout << "Failed to connect" << std::endl;
        return 1;
    }
    
    std::cout << "Running. Press Ctrl+C to stop." << std::endl;
    
    while (should_run) {
        usleep(100000);
    }
    
    polymarket.disconnect();
    std::cout << "Stopped." << std::endl;
    
    return 0;
}
