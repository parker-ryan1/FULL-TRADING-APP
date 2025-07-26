#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <functional>
#include <unordered_map>

namespace hedgefund {
namespace algo {

enum class SignalType {
    BUY,
    SELL,
    HOLD,
    BUY_CALL,
    SELL_CALL,
    BUY_PUT,
    SELL_PUT,
    CLOSE_POSITION
};

enum class StrategyType {
    MOMENTUM,
    MEAN_REVERSION,
    PAIRS_TRADING,
    OPTIONS_STRADDLE,
    OPTIONS_STRANGLE,
    COVERED_CALL,
    PROTECTIVE_PUT,
    IRON_CONDOR,
    BUTTERFLY_SPREAD
};

struct MarketData {
    std::string symbol;
    double price;
    double volume;
    double bid;
    double ask;
    std::chrono::system_clock::time_point timestamp;
    
    // Technical indicators
    double sma_20;
    double sma_50;
    double rsi;
    double bollinger_upper;
    double bollinger_lower;
    double macd;
    double macd_signal;
};

struct TradingSignal {
    std::string strategy_id;
    std::string symbol;
    SignalType signal_type;
    double price;
    double quantity;
    double confidence;
    std::string reason;
    std::chrono::system_clock::time_point timestamp;
    
    // Options specific
    double strike_price = 0.0;
    std::string expiration_date;
    bool is_call = true;
};

struct Position {
    std::string symbol;
    double quantity;
    double average_price;
    double current_price;
    double unrealized_pnl;
    std::chrono::system_clock::time_point entry_time;
    
    // Options specific
    double strike_price = 0.0;
    std::string expiration_date;
    bool is_call = true;
    bool is_option = false;
};

struct StrategyConfig {
    StrategyType type;
    std::string name;
    std::unordered_map<std::string, double> parameters;
    std::vector<std::string> symbols;
    bool enabled;
    double max_position_size;
    double stop_loss_pct;
    double take_profit_pct;
};

class TradingStrategy {
public:
    TradingStrategy(const StrategyConfig& config) : config_(config) {}
    virtual ~TradingStrategy() = default;
    
    virtual std::vector<TradingSignal> generateSignals(const std::vector<MarketData>& market_data) = 0;
    virtual void updatePosition(const Position& position) = 0;
    virtual double calculateRisk(const std::vector<Position>& positions) = 0;
    
    const StrategyConfig& getConfig() const { return config_; }
    
protected:
    StrategyConfig config_;
    std::vector<Position> positions_;
};

class AlgorithmicEngine {
public:
    AlgorithmicEngine();
    ~AlgorithmicEngine();
    
    bool initialize();
    void run();
    void stop();
    
    // Strategy management
    void addStrategy(std::unique_ptr<TradingStrategy> strategy);
    void removeStrategy(const std::string& strategy_id);
    void enableStrategy(const std::string& strategy_id, bool enabled);
    
    // Market data processing
    void processMarketData(const MarketData& data);
    void processSignal(const TradingSignal& signal);
    
    // Risk management
    bool validateSignal(const TradingSignal& signal);
    void updateRiskMetrics();
    
private:
    std::vector<std::unique_ptr<TradingStrategy>> strategies_;
    std::unordered_map<std::string, MarketData> latest_market_data_;
    std::vector<Position> positions_;
    
    bool running_;
    double max_portfolio_risk_;
    double current_portfolio_value_;
    
    void executeSignal(const TradingSignal& signal);
    void updatePositions();
    double calculatePortfolioRisk();
};

} // namespace algo
} // namespace hedgefund