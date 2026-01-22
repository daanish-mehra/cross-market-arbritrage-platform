#include "market_data_client.h"
#include "types.h"
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <curl/curl.h>
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <json/json.h>

const char* PolymarketClient::WEBSOCKET_URL = "wss://clob.polymarket.com";

struct ThreadData {
    PolymarketClient* client;
};

struct MarketInfo {
    std::string token_id;
    std::string event_name;
};

struct CurlWriteData {
    std::string response;
};

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    CurlWriteData* data = (CurlWriteData*)userp;
    data->response.append((char*)contents, total_size);
    return total_size;
}

static std::string http_get(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "";
    }
    
    CurlWriteData write_data;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_data);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        return "";
    }
    
    return write_data.response;
}

static bool parse_orderbook(const std::string& json, double& best_bid, double& best_ask, double& bid_size, double& ask_size) {
    best_bid = 0.0;
    best_ask = 0.0;
    bid_size = 0.0;
    ask_size = 0.0;
    
    // Parse bids array - get first bid
    size_t bids_pos = json.find("\"bids\"");
    if (bids_pos != std::string::npos) {
        size_t array_start = json.find("[", bids_pos);
        if (array_start != std::string::npos && json[array_start + 1] != ']') {
            // Has bids - try quoted string first, then numeric
            size_t price_pos = json.find("\"price\":\"", array_start);
            if (price_pos != std::string::npos) {
                // Quoted string format: "price":"0.5"
                price_pos += 9; // Skip "price":"
                size_t price_end = json.find("\"", price_pos);
                std::string bid_price_str = json.substr(price_pos, price_end - price_pos);
                best_bid = atof(bid_price_str.c_str());
            } else {
                // Numeric format: "price":0.5
                price_pos = json.find("\"price\":", array_start);
                if (price_pos != std::string::npos) {
                    price_pos += 8; // Skip "price":
                    // Find end of number (comma, }, or whitespace)
                    size_t price_end = price_pos;
                    while (price_end < json.length() && 
                           json[price_end] != ',' && 
                           json[price_end] != '}' && 
                           json[price_end] != ' ' &&
                           json[price_end] != '\n' &&
                           json[price_end] != '\r') {
                        price_end++;
                    }
                    std::string bid_price_str = json.substr(price_pos, price_end - price_pos);
                    best_bid = atof(bid_price_str.c_str());
                }
            }
            
            if (best_bid > 0.0) {
                // Parse size - try quoted string first, then numeric
                size_t size_pos = json.find("\"size\":\"", array_start);
                if (size_pos != std::string::npos && size_pos > array_start) {
                    // Quoted string format
                    size_pos += 8; // Skip "size":"
                    size_t size_end = json.find("\"", size_pos);
                    std::string bid_size_str = json.substr(size_pos, size_end - size_pos);
                    bid_size = atof(bid_size_str.c_str());
                } else {
                    // Numeric format: "size":100.0
                    size_pos = json.find("\"size\":", array_start);
                    if (size_pos != std::string::npos && size_pos > array_start) {
                        size_pos += 7; // Skip "size":
                        size_t size_end = size_pos;
                        while (size_end < json.length() && 
                               json[size_end] != ',' && 
                               json[size_end] != '}' && 
                               json[size_end] != ' ' &&
                               json[size_end] != '\n' &&
                               json[size_end] != '\r') {
                            size_end++;
                        }
                        std::string bid_size_str = json.substr(size_pos, size_end - size_pos);
                        bid_size = atof(bid_size_str.c_str());
                    }
                }
            }
        }
    }
    
    // Parse asks array - get first ask
    size_t asks_pos = json.find("\"asks\"");
    if (asks_pos != std::string::npos) {
        size_t array_start = json.find("[", asks_pos);
        if (array_start != std::string::npos && json[array_start + 1] != ']') {
            // Has asks - try quoted string first, then numeric
            size_t price_pos = json.find("\"price\":\"", array_start);
            if (price_pos != std::string::npos) {
                // Quoted string format: "price":"0.5"
                price_pos += 9; // Skip "price":"
                size_t price_end = json.find("\"", price_pos);
                std::string ask_price_str = json.substr(price_pos, price_end - price_pos);
                best_ask = atof(ask_price_str.c_str());
            } else {
                // Numeric format: "price":0.5
                price_pos = json.find("\"price\":", array_start);
                if (price_pos != std::string::npos) {
                    price_pos += 8; // Skip "price":
                    // Find end of number (comma, }, or whitespace)
                    size_t price_end = price_pos;
                    while (price_end < json.length() && 
                           json[price_end] != ',' && 
                           json[price_end] != '}' && 
                           json[price_end] != ' ' &&
                           json[price_end] != '\n' &&
                           json[price_end] != '\r') {
                        price_end++;
                    }
                    std::string ask_price_str = json.substr(price_pos, price_end - price_pos);
                    best_ask = atof(ask_price_str.c_str());
                }
            }
            
            if (best_ask > 0.0) {
                // Parse size - try quoted string first, then numeric
                size_t size_pos = json.find("\"size\":\"", array_start);
                if (size_pos != std::string::npos && size_pos > array_start) {
                    // Quoted string format
                    size_pos += 8; // Skip "size":"
                    size_t size_end = json.find("\"", size_pos);
                    std::string ask_size_str = json.substr(size_pos, size_end - size_pos);
                    ask_size = atof(ask_size_str.c_str());
                } else {
                    // Numeric format: "size":100.0
                    size_pos = json.find("\"size\":", array_start);
                    if (size_pos != std::string::npos && size_pos > array_start) {
                        size_pos += 7; // Skip "size":
                        size_t size_end = size_pos;
                        while (size_end < json.length() && 
                               json[size_end] != ',' && 
                               json[size_end] != '}' && 
                               json[size_end] != ' ' &&
                               json[size_end] != '\n' &&
                               json[size_end] != '\r') {
                            size_end++;
                        }
                        std::string ask_size_str = json.substr(size_pos, size_end - size_pos);
                        ask_size = atof(ask_size_str.c_str());
                    }
                }
            }
        }
    }
    
    return (best_bid > 0.0 || best_ask > 0.0);
}

// Parse Gamma API response and extract market information using jsoncpp
static std::vector<MarketInfo> discover_markets() {
    std::vector<MarketInfo> markets;
    
    // Query Gamma API for active markets (limit to 20 to avoid rate limits)
    std::string url = "https://gamma-api.polymarket.com/events?active=true&closed=false&limit=20";
    std::string response = http_get(url);
    
    if (response.empty()) {
        std::cout << "Gamma API returned empty response" << std::endl;
        return markets;
    }
    
    // Parse JSON using jsoncpp
    Json::Value root;
    Json::Reader reader;
    
    if (!reader.parse(response, root)) {
        std::cout << "Failed to parse Gamma API JSON response" << std::endl;
        return markets;
    }
    
    if (!root.isArray()) {
        std::cout << "Gamma API response is not an array" << std::endl;
        return markets;
    }
    
    // Iterate through events
    for (Json::ArrayIndex i = 0; i < root.size(); i++) {
        const Json::Value& event = root[i];
        if (!event.isObject() || !event.isMember("markets")) {
            continue;
        }
        
        const Json::Value& eventMarkets = event["markets"];
        if (!eventMarkets.isArray()) {
            continue;
        }
        
        // Iterate through markets in this event
        for (Json::ArrayIndex j = 0; j < eventMarkets.size(); j++) {
            const Json::Value& market = eventMarkets[j];
            if (!market.isObject()) {
                continue;
            }
            
            // Check if market is active and has orderbook enabled
            if (!market.isMember("active") || !market["active"].asBool()) {
                continue;
            }
            if (market.isMember("closed") && market["closed"].asBool()) {
                continue;
            }
            if (!market.isMember("enableOrderBook") || !market["enableOrderBook"].asBool()) {
                continue;
            }
            
            // Extract question and clobTokenIds
            if (!market.isMember("question") || !market.isMember("clobTokenIds")) {
                continue;
            }
            
            std::string question = market["question"].asString();
            std::string clobTokenIdsStr = market["clobTokenIds"].asString();
            
            // Parse clobTokenIds JSON string array
            Json::Value tokenIdsArray;
            Json::Reader tokenReader;
            if (!tokenReader.parse(clobTokenIdsStr, tokenIdsArray) || !tokenIdsArray.isArray() || tokenIdsArray.size() == 0) {
                continue;
            }
            
            // Use first token ID (typically the "Yes" outcome)
            std::string tokenId = tokenIdsArray[0].asString();
            
            if (!tokenId.empty() && !question.empty()) {
                MarketInfo info;
                info.token_id = tokenId;
                info.event_name = question;
                markets.push_back(info);
            }
        }
    }
    
    return markets;
}

void* thread_function(void* arg) {
    ThreadData* td = (ThreadData*)arg;
    PolymarketClient* client = td->client;
    
    // Initialize curl (thread-safe)
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    std::vector<MarketInfo> tracked_markets;
    time_t last_discovery = 0;
    const time_t DISCOVERY_INTERVAL = 60; // Discover markets every 60 seconds
    
    std::cout << "Discovering markets from Gamma API..." << std::endl;
    tracked_markets = discover_markets();
    if (tracked_markets.empty()) {
        std::cout << "Warning: No markets discovered. Using fallback market." << std::endl;
        // Fallback to test market
        MarketInfo fallback;
        fallback.token_id = "93233117327291618289066315828674286787516183725243918731390800170422815079307";
        fallback.event_name = "The Fantastic Four: First Steps";
        tracked_markets.push_back(fallback);
    } else {
        std::cout << "Discovered " << tracked_markets.size() << " markets to track." << std::endl;
    }
    last_discovery = time(NULL);
    
    while (client->is_connected()) {
        time_t now = time(NULL);
        
        // Re-discover markets periodically
        if (now - last_discovery >= DISCOVERY_INTERVAL) {
            std::cout << "Re-discovering markets..." << std::endl;
            std::vector<MarketInfo> new_markets = discover_markets();
            if (!new_markets.empty()) {
                tracked_markets = new_markets;
                std::cout << "Now tracking " << tracked_markets.size() << " markets." << std::endl;
            }
            last_discovery = now;
        }
        
        // Poll all tracked markets
        if (client->update_callback != NULL) {
            for (size_t i = 0; i < tracked_markets.size(); i++) {
                const MarketInfo& market = tracked_markets[i];
                
                // Fetch orderbook from Polymarket CLOB API
                std::string url = "https://clob.polymarket.com/book?token_id=" + market.token_id;
                std::string response = http_get(url);
                
                MarketData data;
                data.market = MARKET_POLYMARKET;
                data.market_id = market.token_id;
                data.event_name = market.event_name;
                data.best_bid = 0.0;
                data.best_ask = 0.0;
                data.bid_size = 0.0;
                data.ask_size = 0.0;
                data.is_valid = false;
                
                if (!response.empty()) {
                    // Check for error response
                    if (response.find("\"error\"") != std::string::npos) {
                        // Market has no orderbook - this is normal for some markets
                        data.best_bid = 0.0;
                        data.best_ask = 0.0;
                        data.bid_size = 0.0;
                        data.ask_size = 0.0;
                        data.is_valid = false;
                    } else {
                        double best_bid = 0.0;
                        double best_ask = 0.0;
                        double bid_size = 0.0;
                        double ask_size = 0.0;
                        
                        if (parse_orderbook(response, best_bid, best_ask, bid_size, ask_size)) {
                            data.best_bid = best_bid;
                            data.best_ask = best_ask;
                            data.bid_size = bid_size;
                            data.ask_size = ask_size;
                            data.is_valid = true;
                            std::cout << "Market: " << market.event_name.substr(0, 40) 
                                      << " | Bid: " << best_bid 
                                      << " | Ask: " << best_ask 
                                      << " | Prob: " << ((best_bid + best_ask) / 2.0 * 100.0) << "%" << std::endl;
                        } else {
                            // No valid orderbook data found
                            data.best_bid = 0.0;
                            data.best_ask = 0.0;
                            data.bid_size = 0.0;
                            data.ask_size = 0.0;
                            data.is_valid = false;
                        }
                    }
                } else {
                    data.best_bid = 0.0;
                    data.best_ask = 0.0;
                    data.bid_size = 0.0;
                    data.ask_size = 0.0;
                    data.is_valid = false;
                }
                
                // Send market data even if orderbook is empty (so all markets show up)
                client->update_callback(&data);
                
                // Small delay between markets to avoid rate limiting
                usleep(50000); // 50ms between markets (reduced from 100ms)
            }
        }
        
        // Wait 2 seconds before next poll cycle
        usleep(2000000);
    }
    
    curl_global_cleanup();
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
