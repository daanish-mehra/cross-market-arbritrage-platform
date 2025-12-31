#include "market_data_client.h"
#include "types.h"
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

const char* PolymarketClient::WEBSOCKET_URL = "wss://clob.polymarket.com";

struct ThreadData {
    PolymarketClient* client;
};

void* thread_function(void* arg) {
    ThreadData* td = (ThreadData*)arg;
    PolymarketClient* client = td->client;
    
    srand(time(NULL));
    
    while (client->is_connected()) {
        usleep(100000);
        
        if (client->update_callback != NULL) {
            MarketData data;
            data.market = MARKET_POLYMARKET;
            data.market_id = "polymarket_001";
            data.event_name = "Sample Event";
            
            double base_price = 0.40 + (rand() % 20) / 100.0;
            data.best_bid = base_price - 0.02;
            data.best_ask = base_price + 0.02;
            data.bid_size = 500.0 + (rand() % 1000);
            data.ask_size = 500.0 + (rand() % 1000);
            data.is_valid = true;
            
            client->update_callback(&data);
        }
    }
    
    delete td;
    return NULL;
}

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
    
    ThreadData* td = new ThreadData;
    td->client = this;
    
    pthread_t* thread = new pthread_t;
    if (pthread_create(thread, NULL, thread_function, td) != 0) {
        std::cout << "Failed to start thread" << std::endl;
        connected = false;
        delete td;
        delete thread;
        return false;
    }
    
    worker_thread = thread;
    return true;
}

void PolymarketClient::disconnect() {
    if (!connected) {
        return;
    }
    
    connected = false;
    
    if (worker_thread != NULL) {
        pthread_join(*(pthread_t*)worker_thread, NULL);
        delete (pthread_t*)worker_thread;
        worker_thread = NULL;
    }
}

bool PolymarketClient::is_connected() {
    return connected;
}

void PolymarketClient::set_update_function(void (*func)(MarketData*)) {
    update_callback = func;
}
