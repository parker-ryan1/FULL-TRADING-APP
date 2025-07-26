#pragma once

#include "algo_engine.h"
#include <deque>

namespace hedgefund {
namespace algo {

class MomentumStrategy : public TradingStrategy {
public:
    MomentumStrategy(const StrategyConfig& config);
    
    std::vector<TradingSignal> generateSignals(const std::vector<MarketData>& market_data) override;
    void updatePosition(const Position& position) override;
    double calculateRisk(const std::vector<Position>& positions) override;
    
private:
    struct PriceHistory {
        std::deque<double> prices;
        std::deque<std::chrono::system_clock::time_point> timestamps;
        static const size_t MAX_HISTORY = 50;
    };
    
    std::unordered_map<std::string, PriceHistory> price_history_;
    
    double calculateMomentum(const std::string& symbol, int lookback_periods);
    double calculateVolatility(const std::string& symbol, int lookback_periods);
    bool isBreakout(const MarketData& data);
    TradingSignal createSignal(const std::string& symbol, SignalType type, double price, double confidence, const std::string& reason);
};

} // namespace algo
} // namespace hedgefund