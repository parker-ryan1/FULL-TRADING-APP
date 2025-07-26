#include "momentum_strategy.h"
#include <cmath>
#include <numeric>
#include <algorithm>
#include <iostream>

namespace hedgefund {
namespace algo {

MomentumStrategy::MomentumStrategy(const StrategyConfig& config) : TradingStrategy(config) {
    std::cout << "Initialized Momentum Strategy: " << config.name << std::endl;
}

std::vector<TradingSignal> MomentumStrategy::generateSignals(const std::vector<MarketData>& market_data) {
    std::vector<TradingSignal> signals;
    
    for (const auto& data : market_data) {
        // Update price history
        auto& history = price_history_[data.symbol];
        history.prices.push_back(data.price);
        history.timestamps.push_back(data.timestamp);
        
        if (history.prices.size() > PriceHistory::MAX_HISTORY) {
            history.prices.pop_front();
            history.timestamps.pop_front();
        }
        
        // Need at least 20 data points for momentum calculation
        if (history.prices.size() < 20) continue;
        
        // Calculate momentum indicators
        double short_momentum = calculateMomentum(data.symbol, 5);  // 5-period momentum
        double long_momentum = calculateMomentum(data.symbol, 20);  // 20-period momentum
        double volatility = calculateVolatility(data.symbol, 20);
        
        // Momentum strategy logic
        double momentum_threshold = config_.parameters.at("momentum_threshold"); // e.g., 0.02 (2%)
        double volatility_threshold = config_.parameters.at("volatility_threshold"); // e.g., 0.03 (3%)
        
        // Strong upward momentum
        if (short_momentum > momentum_threshold && long_momentum > 0 && volatility < volatility_threshold) {
            if (data.rsi < 70) { // Not overbought
                double confidence = std::min(0.95, 0.5 + (short_momentum * 10));
                signals.push_back(createSignal(data.symbol, SignalType::BUY, data.price, confidence, 
                    "Strong upward momentum detected"));
            }
        }
        
        // Strong downward momentum
        else if (short_momentum < -momentum_threshold && long_momentum < 0 && volatility < volatility_threshold) {
            if (data.rsi > 30) { // Not oversold
                double confidence = std::min(0.95, 0.5 + (std::abs(short_momentum) * 10));
                signals.push_back(createSignal(data.symbol, SignalType::SELL, data.price, confidence, 
                    "Strong downward momentum detected"));
            }
        }
        
        // Breakout detection
        else if (isBreakout(data)) {
            double breakout_confidence = 0.75;
            if (data.price > data.bollinger_upper) {
                signals.push_back(createSignal(data.symbol, SignalType::BUY, data.price, breakout_confidence, 
                    "Bollinger Band breakout (upper)"));
            } else if (data.price < data.bollinger_lower) {
                signals.push_back(createSignal(data.symbol, SignalType::SELL, data.price, breakout_confidence, 
                    "Bollinger Band breakout (lower)"));
            }
        }
        
        // Mean reversion after extreme momentum
        else if (std::abs(short_momentum) > momentum_threshold * 2 && volatility > volatility_threshold) {
            double reversion_confidence = 0.65;
            if (short_momentum > 0 && data.rsi > 80) {
                signals.push_back(createSignal(data.symbol, SignalType::SELL, data.price, reversion_confidence, 
                    "Mean reversion after extreme upward momentum"));
            } else if (short_momentum < 0 && data.rsi < 20) {
                signals.push_back(createSignal(data.symbol, SignalType::BUY, data.price, reversion_confidence, 
                    "Mean reversion after extreme downward momentum"));
            }
        }
    }
    
    return signals;
}

void MomentumStrategy::updatePosition(const Position& position) {
    // Update internal position tracking
    bool found = false;
    for (auto& pos : positions_) {
        if (pos.symbol == position.symbol) {
            pos = position;
            found = true;
            break;
        }
    }
    
    if (!found) {
        positions_.push_back(position);
    }
}

double MomentumStrategy::calculateRisk(const std::vector<Position>& positions) {
    double total_risk = 0.0;
    
    for (const auto& position : positions) {
        // Calculate position risk based on volatility and position size
        double position_value = std::abs(position.quantity * position.current_price);
        
        auto it = price_history_.find(position.symbol);
        if (it != price_history_.end()) {
            double volatility = calculateVolatility(position.symbol, 20);
            double position_risk = position_value * volatility;
            total_risk += position_risk;
        }
    }
    
    return total_risk;
}

double MomentumStrategy::calculateMomentum(const std::string& symbol, int lookback_periods) {
    auto it = price_history_.find(symbol);
    if (it == price_history_.end() || it->second.prices.size() < lookback_periods + 1) {
        return 0.0;
    }
    
    const auto& prices = it->second.prices;
    double current_price = prices.back();
    double past_price = prices[prices.size() - lookback_periods - 1];
    
    return (current_price - past_price) / past_price;
}

double MomentumStrategy::calculateVolatility(const std::string& symbol, int lookback_periods) {
    auto it = price_history_.find(symbol);
    if (it == price_history_.end() || it->second.prices.size() < lookback_periods) {
        return 0.0;
    }
    
    const auto& prices = it->second.prices;
    std::vector<double> returns;
    
    for (size_t i = prices.size() - lookback_periods; i < prices.size() - 1; ++i) {
        double return_val = (prices[i + 1] - prices[i]) / prices[i];
        returns.push_back(return_val);
    }
    
    if (returns.empty()) return 0.0;
    
    double mean_return = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double variance = 0.0;
    
    for (double ret : returns) {
        variance += (ret - mean_return) * (ret - mean_return);
    }
    
    variance /= returns.size();
    return std::sqrt(variance);
}

bool MomentumStrategy::isBreakout(const MarketData& data) {
    // Simple breakout detection using Bollinger Bands
    double band_width = data.bollinger_upper - data.bollinger_lower;
    double price_position = (data.price - data.bollinger_lower) / band_width;
    
    // Breakout if price is outside bands with sufficient volume
    return (price_position > 1.0 || price_position < 0.0) && data.volume > 1000;
}

TradingSignal MomentumStrategy::createSignal(const std::string& symbol, SignalType type, double price, 
                                           double confidence, const std::string& reason) {
    TradingSignal signal;
    signal.strategy_id = config_.name;
    signal.symbol = symbol;
    signal.signal_type = type;
    signal.price = price;
    signal.quantity = config_.max_position_size;
    signal.confidence = confidence;
    signal.reason = reason;
    signal.timestamp = std::chrono::system_clock::now();
    
    return signal;
}

} // namespace algo
} // namespace hedgefund