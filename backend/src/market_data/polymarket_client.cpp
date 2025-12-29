#include "market_data_client.h"
#include "types.h"
#include <iostream>

const char* PolymarketClient::WEBSOCKET_URL = "wss://clob.polymarket.com";

PolymarketClient::PolymarketClient() {
    connected = false;
    update_callback = NULL;
    worker_thread = NULL;
}

PolymarketClient::~PolymarketClient() {
    disconnect();
}

bool PolymarketClient::connect() {
    if (connected) {
        return true;
    }
    
    std::cout << "Connecting to Polymarket..." << std::endl;
    connected = true;
    return true;
}

void PolymarketClient::disconnect() {
    if (!connected) {
        return;
    }
    connected = false;
}

bool PolymarketClient::is_connected() {
    return connected;
}

void PolymarketClient::set_update_function(void (*func)(MarketData*)) {
    update_callback = func;
}
