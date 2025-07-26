#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <queue>
#include <memory>
#include <functional>

namespace hedgefund {
namespace marketdata {

struct PolygonTicker {
    std::string symbol;
    double price;
    double change;
    double change_percent;
    double volume;
    double high;
    double low;
    double open;
    double close;
    std::string timestamp;
};

struct PolygonAgg {
    std::string symbol;
    double open;
    double high;
    double low;
    double close;
    double volume;
    double vwap;
    int64_t timestamp;
    int transactions;
};

struct PolygonOptionsContract {
    std::string contract_type; // "call" or "put"
    std::string expiration_date;
    double strike_price;
    std::string ticker;
    std::string underlying_ticker;
    double last_quote_price;
    double implied_volatility;
    double delta;
    double gamma;
    double theta;
    double vega;
    double rho;
};

class RateLimiter {
public:
    RateLimiter(int max_calls, int time_window_seconds);
    bool canMakeCall();
    void recordCall();
    int getCallsRemaining();
    int getSecondsUntilReset();

private:
    int max_calls_;
    int time_window_seconds_;
    std::queue<std::chrono::steady_clock::time_point> call_times_;
    void cleanOldCalls();
};

class PolygonClient {
public:
    PolygonClient(const std::string& api_key);
    ~PolygonClient();

    // Market data methods
    bool getStockTicker(const std::string& symbol, PolygonTicker& ticker);
    bool getStockAggregates(const std::string& symbol, const std::string& timespan, 
                           const std::string& from, const std::string& to, 
                           std::vector<PolygonAgg>& aggregates);
    bool getOptionsContracts(const std::string& underlying_symbol, 
                           std::vector<PolygonOptionsContract>& contracts);
    bool getLastTrade(const std::string& symbol, PolygonTicker& ticker);

    // Rate limiting info
    int getRemainingCalls();
    int getSecondsUntilReset();
    bool isRateLimited();

    // Callback for real-time data
    void setDataCallback(std::function<void(const PolygonTicker&)> callback);

private:
    std::string api_key_;
    std::unique_ptr<RateLimiter> rate_limiter_;
    std::function<void(const PolygonTicker&)> data_callback_;

    // HTTP client methods
    std::string makeHttpRequest(const std::string& url);
    std::string buildUrl(const std::string& endpoint, const std::string& params = "");
    bool parseTickerResponse(const std::string& json, PolygonTicker& ticker);
    bool parseAggregatesResponse(const std::string& json, std::vector<PolygonAgg>& aggregates);
    bool parseOptionsResponse(const std::string& json, std::vector<PolygonOptionsContract>& contracts);
};

} // namespace marketdata
} // namespace hedgefund