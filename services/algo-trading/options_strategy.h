#pragma once

#include "algo_engine.h"
#include "../options/black_scholes.h"

namespace hedgefund {
namespace algo {

class OptionsStrategy : public TradingStrategy {
public:
    OptionsStrategy(const StrategyConfig& config);
    
    std::vector<TradingSignal> generateSignals(const std::vector<MarketData>& market_data) override;
    void updatePosition(const Position& position) override;
    double calculateRisk(const std::vector<Position>& positions) override;
    
private:
    struct OptionsChain {
        std::string underlying_symbol;
        std::vector<double> strike_prices;
        std::string expiration_date;
        std::unordered_map<double, double> call_prices;
        std::unordered_map<double, double> put_prices;
        std::unordered_map<double, double> implied_vols;
    };
    
    std::unordered_map<std::string, OptionsChain> options_chains_;
    
    // Strategy implementations
    std::vector<TradingSignal> generateStraddleSignals(const MarketData& data);
    std::vector<TradingSignal> generateStrangleSignals(const MarketData& data);
    std::vector<TradingSignal> generateCoveredCallSignals(const MarketData& data);
    std::vector<TradingSignal> generateProtectivePutSignals(const MarketData& data);
    std::vector<TradingSignal> generateIronCondorSignals(const MarketData& data);
    std::vector<TradingSignal> generateButterflySignals(const MarketData& data);
    
    // Helper functions
    double calculateImpliedVolatility(const std::string& symbol, double strike, bool is_call, double market_price);
    double calculateOptionPrice(const std::string& symbol, double strike, bool is_call, const std::string& expiration);
    bool isHighVolatility(const std::string& symbol);
    bool isLowVolatility(const std::string& symbol);
    double getTimeToExpiration(const std::string& expiration_date);
    std::vector<double> getATMStrikes(const std::string& symbol, double spot_price);
    
    TradingSignal createOptionsSignal(const std::string& symbol, SignalType type, double strike, 
                                    bool is_call, const std::string& expiration, double price, 
                                    double confidence, const std::string& reason);
};

} // namespace algo
} // namespace hedgefund