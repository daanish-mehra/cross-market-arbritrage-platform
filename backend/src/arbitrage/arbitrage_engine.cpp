#include "arbitrage_engine.h"
#include "types.h"
#include <map>
#include <string>
#include <vector>
#include <pthread.h>

struct MarketDataMap {
    std::map<std::string, MarketData> data;
    pthread_mutex_t mutex;
    
    MarketDataMap() {
        pthread_mutex_init(&mutex, NULL);
    }
    
    ~MarketDataMap() {
        pthread_mutex_destroy(&mutex);
    }
};

ArbitrageEngine::ArbitrageEngine(Config* config) {
    this->config = config;
    this->opportunity_callback = NULL;
    this->market_data_map = new MarketDataMap();
}

ArbitrageEngine::~ArbitrageEngine() {
    delete (MarketDataMap*)market_data_map;
}

void ArbitrageEngine::update_market_data(MarketData* data) {
    if (data == NULL || !data->is_valid) {
        return;
    }
    
    MarketDataMap* mdm = (MarketDataMap*)market_data_map;
    pthread_mutex_lock(&mdm->mutex);
    mdm->data[data->market_id] = *data;
    pthread_mutex_unlock(&mdm->mutex);
    
    check_for_opportunities();
}

void ArbitrageEngine::set_opportunity_function(void (*func)(ArbitrageOpportunity*)) {
    opportunity_callback = func;
}

void ArbitrageEngine::check_for_opportunities() {
    if (opportunity_callback == NULL) {
        return;
    }
    
    MarketDataMap* mdm = (MarketDataMap*)market_data_map;
    pthread_mutex_lock(&mdm->mutex);
    
    // O(n): Group markets by event_name first
    // This reduces complexity from O(n²) to O(n + k²×e)
    // where k = markets per event (typically 2-3), e = unique events
    std::map<std::string, std::vector<MarketData*>> events_map;
    for (std::map<std::string, MarketData>::iterator it = mdm->data.begin(); 
         it != mdm->data.end(); ++it) {
        events_map[it->second.event_name].push_back(&it->second);
    }
    
    pthread_mutex_unlock(&mdm->mutex);
    
    // O(k²×e): Only compare markets within the same event
    // For each event, compare pairs of markets (typically 2-3 markets per event)
    for (std::map<std::string, std::vector<MarketData*>>::iterator event_it = events_map.begin();
         event_it != events_map.end(); ++event_it) {
        
        std::vector<MarketData*>& markets = event_it->second;
        
        // Skip if less than 2 markets for this event
        if (markets.size() < 2) {
            continue;
        }
        
        // Compare all pairs of markets for this event
        for (size_t i = 0; i < markets.size(); i++) {
            for (size_t j = i + 1; j < markets.size(); j++) {
                MarketData* data1 = markets[i];
                MarketData* data2 = markets[j];
                
                // Skip if same market (shouldn't happen, but safety check)
                if (data1->market == data2->market) {
                    continue;
                }
                
                double profit = compute_profit(data1, data2);
                
                if (profit > config->min_profit_threshold) {
                    ArbitrageOpportunity opp;
                    opp.event_id = data1->event_name;
                    opp.buy_market = data1->market;
                    opp.sell_market = data2->market;
                    opp.buy_price = data1->best_ask;
                    opp.sell_price = data2->best_bid;
                    opp.profit_percentage = profit;
                    opp.max_size = compute_max_size(data1, data2);
                    
                    if (opportunity_callback != NULL) {
                        opportunity_callback(&opp);
                    }
                }
            }
        }
    }
}

double ArbitrageEngine::compute_profit(MarketData* buy, MarketData* sell) {
    if (buy == NULL || sell == NULL) {
        return 0.0;
    }
    
    if (buy->best_ask >= sell->best_bid) {
        return 0.0;
    }
    
    double buy_price = buy->best_ask;
    double sell_price = sell->best_bid;
    
    double buy_fee = buy_price * 0.02;
    double sell_fee = sell_price * 0.02;
    
    double net_profit = sell_price - buy_price - buy_fee - sell_fee;
    
    if (net_profit <= 0.0) {
        return 0.0;
    }
    
    double profit_ratio = net_profit / buy_price;
    return profit_ratio;
}

double ArbitrageEngine::compute_max_size(MarketData* buy, MarketData* sell) {
    if (buy == NULL || sell == NULL) {
        return 0.0;
    }
    
    double buy_max = buy->ask_size;
    double sell_max = sell->bid_size;
    
    if (buy_max < sell_max) {
        return buy_max;
    } else {
        return sell_max;
    }
}
