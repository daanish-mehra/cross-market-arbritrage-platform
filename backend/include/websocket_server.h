#pragma once

#include "types.h"
#include <string>
#include <functional>
#include <cstddef>

class WebSocketServer {
public:
    WebSocketServer(int port);
    ~WebSocketServer();
    
    bool start();
    void stop();
    bool is_running();
    
    void broadcast_opportunity(ArbitrageOpportunity* opp);
    void broadcast_market_data(MarketData* data);
    
    void set_on_connect(std::function<void(int)> callback);
    void set_on_disconnect(std::function<void(int)> callback);
    
    void server_loop();
    
private:
    void handle_client(int client_fd);
    bool handle_websocket_upgrade(int client_fd, const std::string& request);
    void send_message(int client_fd, const std::string& message);
    std::string create_opportunity_json(ArbitrageOpportunity* opp);
    std::string create_market_data_json(MarketData* data);
    
    int port;
    int server_fd;
    bool running;
    void* server_thread;
    std::function<void(int)> on_connect;
    std::function<void(int)> on_disconnect;
};

