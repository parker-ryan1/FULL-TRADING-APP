#include "polygon_client.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <algorithm>
#include <ctime>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#else
#include <curl/curl.h>
#endif

namespace hedgefund {
namespace marketdata {

// Rate Limiter Implementation
RateLimiter::RateLimiter(int max_calls, int time_window_seconds) 
    : max_calls_(max_calls), time_window_seconds_(time_window_seconds) {}

bool RateLimiter::canMakeCall() {
    cleanOldCalls();
    return call_times_.size() < static_cast<size_t>(max_calls_);
}

void RateLimiter::recordCall() {
    call_times_.push(std::chrono::steady_clock::now());
}

int RateLimiter::getCallsRemaining() {
    cleanOldCalls();
    return max_calls_ - static_cast<int>(call_times_.size());
}

int RateLimiter::getSecondsUntilReset() {
    if (call_times_.empty()) return 0;
    
    auto oldest_call = call_times_.front();
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - oldest_call).count();
    
    return std::max(0, time_window_seconds_ - static_cast<int>(elapsed));
}

void RateLimiter::cleanOldCalls() {
    auto now = std::chrono::steady_clock::now();
    auto cutoff = now - std::chrono::seconds(time_window_seconds_);
    
    while (!call_times_.empty() && call_times_.front() < cutoff) {
        call_times_.pop();
    }
}

// Polygon Client Implementation
PolygonClient::PolygonClient(const std::string& api_key) 
    : api_key_(api_key) {
    // 4 calls per minute = 4 calls per 60 seconds
    rate_limiter_ = std::make_unique<RateLimiter>(4, 60);
    
    std::cout << "Initialized Polygon.io client with API key: " 
              << api_key.substr(0, 8) << "..." << std::endl;
}

PolygonClient::~PolygonClient() = default;

bool PolygonClient::getStockTicker(const std::string& symbol, PolygonTicker& ticker) {
    if (!rate_limiter_->canMakeCall()) {
        std::cout << "Rate limit exceeded. Wait " << rate_limiter_->getSecondsUntilReset() 
                  << " seconds." << std::endl;
        return false;
    }

    std::string url = buildUrl("/v2/snapshot/locale/us/markets/stocks/tickers/" + symbol);
    std::string response = makeHttpRequest(url);
    
    if (response.empty()) {
        std::cout << "Failed to get response for " << symbol << std::endl;
        return false;
    }

    rate_limiter_->recordCall();
    
    if (parseTickerResponse(response, ticker)) {
        ticker.symbol = symbol;
        std::cout << "Retrieved ticker data for " << symbol << ": $" << ticker.price << std::endl;
        
        // Trigger callback if set
        if (data_callback_) {
            data_callback_(ticker);
        }
        
        return true;
    }

    return false;
}

bool PolygonClient::getStockAggregates(const std::string& symbol, const std::string& timespan,
                                     const std::string& from, const std::string& to,
                                     std::vector<PolygonAgg>& aggregates) {
    if (!rate_limiter_->canMakeCall()) {
        std::cout << "Rate limit exceeded. Wait " << rate_limiter_->getSecondsUntilReset() 
                  << " seconds." << std::endl;
        return false;
    }

    std::string params = "?multiplier=1&timespan=" + timespan + "&from=" + from + "&to=" + to;
    std::string url = buildUrl("/v2/aggs/ticker/" + symbol + "/range/1/" + timespan + "/" + from + "/" + to);
    std::string response = makeHttpRequest(url);
    
    if (response.empty()) {
        return false;
    }

    rate_limiter_->recordCall();
    
    if (parseAggregatesResponse(response, aggregates)) {
        std::cout << "Retrieved " << aggregates.size() << " aggregates for " << symbol << std::endl;
        return true;
    }

    return false;
}

bool PolygonClient::getOptionsContracts(const std::string& underlying_symbol,
                                       std::vector<PolygonOptionsContract>& contracts) {
    if (!rate_limiter_->canMakeCall()) {
        std::cout << "Rate limit exceeded. Wait " << rate_limiter_->getSecondsUntilReset() 
                  << " seconds." << std::endl;
        return false;
    }

    std::string params = "?underlying_ticker=" + underlying_symbol + "&limit=100";
    std::string url = buildUrl("/v3/reference/options/contracts" + params);
    std::string response = makeHttpRequest(url);
    
    if (response.empty()) {
        return false;
    }

    rate_limiter_->recordCall();
    
    if (parseOptionsResponse(response, contracts)) {
        std::cout << "Retrieved " << contracts.size() << " options contracts for " 
                  << underlying_symbol << std::endl;
        return true;
    }

    return false;
}

bool PolygonClient::getLastTrade(const std::string& symbol, PolygonTicker& ticker) {
    if (!rate_limiter_->canMakeCall()) {
        std::cout << "Rate limit exceeded. Wait " << rate_limiter_->getSecondsUntilReset() 
                  << " seconds." << std::endl;
        return false;
    }

    std::string url = buildUrl("/v2/last/trade/" + symbol);
    std::string response = makeHttpRequest(url);
    
    if (response.empty()) {
        return false;
    }

    rate_limiter_->recordCall();
    
    // Simplified parsing for last trade
    ticker.symbol = symbol;
    ticker.price = 150.0 + (rand() % 1000 - 500) / 100.0; // Simulated for demo
    ticker.volume = 1000 + (rand() % 10000);
    
    std::cout << "Retrieved last trade for " << symbol << ": $" << ticker.price << std::endl;
    
    if (data_callback_) {
        data_callback_(ticker);
    }
    
    return true;
}

int PolygonClient::getRemainingCalls() {
    return rate_limiter_->getCallsRemaining();
}

int PolygonClient::getSecondsUntilReset() {
    return rate_limiter_->getSecondsUntilReset();
}

bool PolygonClient::isRateLimited() {
    return !rate_limiter_->canMakeCall();
}

void PolygonClient::setDataCallback(std::function<void(const PolygonTicker&)> callback) {
    data_callback_ = callback;
}

std::string PolygonClient::makeHttpRequest(const std::string& url) {
    std::cout << "Making HTTP request to: " << url << std::endl;
    
    // For demonstration, we'll simulate API responses
    // In production, this would make actual HTTP requests to Polygon.io
    
    // Simulate network delay
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Return simulated JSON response
    return R"({
        "status": "OK",
        "results": {
            "ticker": {
                "day": {
                    "c": 150.25,
                    "h": 152.10,
                    "l": 148.90,
                    "o": 149.50,
                    "v": 1250000
                },
                "lastQuote": {
                    "P": 150.25,
                    "p": 150.20
                },
                "prevDay": {
                    "c": 149.80
                }
            }
        }
    })";
}

std::string PolygonClient::buildUrl(const std::string& endpoint, const std::string& params) {
    std::string base_url = "https://api.polygon.io";
    std::string url = base_url + endpoint;
    
    if (!params.empty()) {
        url += (params[0] == '?' ? "" : "?") + params;
        url += "&apikey=" + api_key_;
    } else {
        url += "?apikey=" + api_key_;
    }
    
    return url;
}

bool PolygonClient::parseTickerResponse(const std::string& json, PolygonTicker& ticker) {
    // Simplified JSON parsing for demonstration
    // In production, would use a proper JSON library like nlohmann/json
    
    ticker.price = 150.25;
    ticker.high = 152.10;
    ticker.low = 148.90;
    ticker.open = 149.50;
    ticker.volume = 1250000;
    ticker.change = 0.45;
    ticker.change_percent = 0.30;
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ticker.timestamp = ss.str();
    
    return true;
}

bool PolygonClient::parseAggregatesResponse(const std::string& json, std::vector<PolygonAgg>& aggregates) {
    // Simplified parsing - generate sample aggregates
    aggregates.clear();
    
    for (int i = 0; i < 10; i++) {
        PolygonAgg agg;
        agg.open = 150.0 + i * 0.5;
        agg.high = agg.open + 2.0;
        agg.low = agg.open - 1.5;
        agg.close = agg.open + (rand() % 200 - 100) / 100.0;
        agg.volume = 100000 + rand() % 500000;
        agg.vwap = (agg.high + agg.low + agg.close) / 3.0;
        agg.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() - (i * 60000);
        agg.transactions = 1000 + rand() % 5000;
        
        aggregates.push_back(agg);
    }
    
    return true;
}

bool PolygonClient::parseOptionsResponse(const std::string& json, std::vector<PolygonOptionsContract>& contracts) {
    // Simplified parsing - generate sample options contracts
    contracts.clear();
    
    std::vector<std::string> expirations = {"2024-03-15", "2024-04-19", "2024-05-17"};
    std::vector<double> strikes = {140, 145, 150, 155, 160};
    
    for (const auto& exp : expirations) {
        for (double strike : strikes) {
            // Call option
            PolygonOptionsContract call;
            call.contract_type = "call";
            call.expiration_date = exp;
            call.strike_price = strike;
            call.ticker = "AAPL" + exp + "C" + std::to_string(static_cast<int>(strike));
            call.underlying_ticker = "AAPL";
            call.last_quote_price = std::max(0.1, 150.0 - strike + 5.0);
            call.implied_volatility = 0.20 + (std::abs(strike - 150.0) / 150.0) * 0.1;
            call.delta = strike < 150 ? 0.6 : 0.4;
            call.gamma = 0.05;
            call.theta = -0.02;
            call.vega = 0.15;
            call.rho = 0.08;
            contracts.push_back(call);
            
            // Put option
            PolygonOptionsContract put;
            put.contract_type = "put";
            put.expiration_date = exp;
            put.strike_price = strike;
            put.ticker = "AAPL" + exp + "P" + std::to_string(static_cast<int>(strike));
            put.underlying_ticker = "AAPL";
            put.last_quote_price = std::max(0.1, strike - 150.0 + 5.0);
            put.implied_volatility = 0.20 + (std::abs(strike - 150.0) / 150.0) * 0.1;
            put.delta = strike > 150 ? -0.6 : -0.4;
            put.gamma = 0.05;
            put.theta = -0.02;
            put.vega = 0.15;
            put.rho = -0.08;
            contracts.push_back(put);
        }
    }
    
    return true;
}

} // namespace marketdata
} // namespace hedgefund