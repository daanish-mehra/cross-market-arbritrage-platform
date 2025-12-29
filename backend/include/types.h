#pragma once

#include <string>

enum Market {
    MARKET_POLYMARKET,
    MARKET_KALSHI,
    MARKET_PREDICTIT
};

struct MarketData {
    std::string market_id;
    int market;
    std::string event_name;
    double best_bid;
    double best_ask;
    double bid_size;
    double ask_size;
    bool is_valid;
    
    MarketData() {
        market_id = "";
        market = MARKET_POLYMARKET;
        event_name = "";
        best_bid = 0.0;
        best_ask = 0.0;
        bid_size = 0.0;
        ask_size = 0.0;
        is_valid = false;
    }
};

struct ArbitrageOpportunity {
    std::string event_id;
    int buy_market;
    int sell_market;
    double buy_price;
    double sell_price;
    double profit_percentage;
    double max_size;
    
    ArbitrageOpportunity() {
        event_id = "";
        buy_market = MARKET_POLYMARKET;
        sell_market = MARKET_POLYMARKET;
        buy_price = 0.0;
        sell_price = 0.0;
        profit_percentage = 0.0;
        max_size = 0.0;
    }
};

struct Config {
    double min_profit_threshold;
    int update_interval_ms;
    std::string websocket_port;
    bool enable_execution;
    
    Config() {
        min_profit_threshold = 0.01;
        update_interval_ms = 100;
        websocket_port = "8080";
        enable_execution = false;
    }
};
