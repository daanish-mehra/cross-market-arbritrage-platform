#pragma once

#include "types.h"

class PolymarketClient {
public:
    PolymarketClient();
    ~PolymarketClient();
    
    bool connect();
    void disconnect();
    bool is_connected();
    void set_update_function(void (*func)(MarketData*));
    
    bool connected;
    void (*update_callback)(MarketData*);
    
private:
    void* worker_thread;
    static const char* WEBSOCKET_URL;
};

