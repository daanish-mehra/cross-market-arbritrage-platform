#include "types.h"
#include "market_data_client.h"
#include "arbitrage_engine.h"
#include "websocket_server.h"
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

bool should_run = true;
ArbitrageEngine* global_engine = NULL;
WebSocketServer* global_server = NULL;

void handle_signal(int sig) {
    std::cout << "\nShutting down..." << std::endl;
    should_run = false;
}

void on_opportunity(ArbitrageOpportunity* opp) {
    double profit_pct = opp->profit_percentage * 100.0;
    std::cout << "Opportunity: " << opp->event_id << " - " << profit_pct 
              << "% profit, buy at " << opp->buy_price << " sell at " 
              << opp->sell_price << std::endl;
    
    if (global_server != NULL) {
        global_server->broadcast_opportunity(opp);
    }
}

void on_market_update(MarketData* data) {
    if (global_engine != NULL) {
        global_engine->update_market_data(data);
    }
    
    if (global_server != NULL) {
        global_server->broadcast_market_data(data);
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
    
    int ws_port = 8080;
    if (argc > 1) {
        ws_port = atoi(argv[1]);
    }
    
    WebSocketServer ws_server(ws_port);
    if (!ws_server.start()) {
        std::cout << "Failed to start WebSocket server" << std::endl;
        return 1;
    }
    global_server = &ws_server;
    
    PolymarketClient polymarket;
    polymarket.set_update_function(on_market_update);
    
    std::cout << "Connecting to Polymarket..." << std::endl;
    if (!polymarket.connect()) {
        std::cout << "Failed to connect" << std::endl;
        ws_server.stop();
        return 1;
    }
    
    std::cout << "Running. WebSocket server on port " << ws_port << ". Press Ctrl+C to stop." << std::endl;
    
    while (should_run) {
        usleep(100000);
    }
    
    polymarket.disconnect();
    ws_server.stop();
    std::cout << "Stopped." << std::endl;
    
    return 0;
}
