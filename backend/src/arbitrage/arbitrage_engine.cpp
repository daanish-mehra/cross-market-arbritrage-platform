#include "arbitrage_engine.h"
#include "types.h"
#include <map>
#include <string>
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
    
    std::map<std::string, MarketData>::iterator it1;
    std::map<std::string, MarketData>::iterator it2;
    
    for (it1 = mdm->data.begin(); it1 != mdm->data.end(); ++it1) {
        for (it2 = mdm->data.begin(); it2 != mdm->data.end(); ++it2) {
            if (it1 == it2) {
                continue;
            }
            
            MarketData* data1 = &it1->second;
            MarketData* data2 = &it2->second;
            
            if (data1->event_name != data2->event_name) {
                continue;
            }
            
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
                
                pthread_mutex_unlock(&mdm->mutex);
                if (opportunity_callback != NULL) {
                    opportunity_callback(&opp);
                }
                pthread_mutex_lock(&mdm->mutex);
            }
        }
    }
    
    pthread_mutex_unlock(&mdm->mutex);
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
