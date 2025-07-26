#include "algo_engine.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <thread>

namespace hedgefund {
namespace algo {

AlgorithmicEngine::AlgorithmicEngine() 
    : running_(false), max_portfolio_risk_(0.02), current_portfolio_value_(1000000.0) {}

AlgorithmicEngine::~AlgorithmicEngine() {
    stop();
}

bool AlgorithmicEngine::initialize() {
    std::cout << "Initializing Algorithmic Trading Engine..." << std::endl;
    
    // Initialize risk parameters
    max_portfolio_risk_ = 0.02; // 2% max portfolio risk
    current_portfolio_value_ = 1000000.0; // $1M starting capital
    
    std::cout << "Algorithmic Engine initialized successfully" << std::endl;
    return true;
}

void AlgorithmicEngine::run() {
    running_ = true;
    std::cout << "Algorithmic Trading Engine started" << std::endl;
    
    while (running_) {
        // Process market data and generate signals
        for (auto& strategy : strategies_) {
            if (!strategy->getConfig().enabled) continue;
            
            std::vector<MarketData> relevant_data;
            for (const auto& symbol : strategy->getConfig().symbols) {
                auto it = latest_market_data_.find(symbol);
                if (it != latest_market_data_.end()) {
                    relevant_data.push_back(it->second);
                }
            }
            
            if (!relevant_data.empty()) {
                auto signals = strategy->generateSignals(relevant_data);
                for (const auto& signal : signals) {
                    processSignal(signal);
                }
            }
        }
        
        updatePositions();
        updateRiskMetrics();
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void AlgorithmicEngine::stop() {
    running_ = false;
    std::cout << "Algorithmic Trading Engine stopped" << std::endl;
}

void AlgorithmicEngine::addStrategy(std::unique_ptr<TradingStrategy> strategy) {
    std::cout << "Adding strategy: " << strategy->getConfig().name << std::endl;
    strategies_.push_back(std::move(strategy));
}

void AlgorithmicEngine::removeStrategy(const std::string& strategy_id) {
    strategies_.erase(
        std::remove_if(strategies_.begin(), strategies_.end(),
            [&strategy_id](const auto& strategy) {
                return strategy->getConfig().name == strategy_id;
            }),
        strategies_.end()
    );
}

void AlgorithmicEngine::enableStrategy(const std::string& strategy_id, bool enabled) {
    for (auto& strategy : strategies_) {
        if (strategy->getConfig().name == strategy_id) {
            const_cast<StrategyConfig&>(strategy->getConfig()).enabled = enabled;
            std::cout << "Strategy " << strategy_id << (enabled ? " enabled" : " disabled") << std::endl;
            break;
        }
    }
}

void AlgorithmicEngine::processMarketData(const MarketData& data) {
    latest_market_data_[data.symbol] = data;
    
    // Calculate technical indicators (simplified)
    MarketData& updated_data = latest_market_data_[data.symbol];
    
    // Simple moving averages (would need historical data in real implementation)
    updated_data.sma_20 = data.price; // Simplified
    updated_data.sma_50 = data.price; // Simplified
    
    // RSI calculation (simplified)
    updated_data.rsi = 50.0; // Neutral RSI
    
    // Bollinger Bands (simplified)
    double volatility = data.price * 0.02; // 2% volatility assumption
    updated_data.bollinger_upper = data.price + (2 * volatility);
    updated_data.bollinger_lower = data.price - (2 * volatility);
    
    // MACD (simplified)
    updated_data.macd = 0.0;
    updated_data.macd_signal = 0.0;
}

void AlgorithmicEngine::processSignal(const TradingSignal& signal) {
    if (!validateSignal(signal)) {
        std::cout << "Signal validation failed for " << signal.symbol << std::endl;
        return;
    }
    
    std::cout << "Processing signal: " << signal.symbol 
              << " " << static_cast<int>(signal.signal_type)
              << " @ " << signal.price 
              << " (confidence: " << signal.confidence << ")" << std::endl;
    
    executeSignal(signal);
}

bool AlgorithmicEngine::validateSignal(const TradingSignal& signal) {
    // Risk validation
    double position_value = signal.price * signal.quantity;
    double portfolio_risk = position_value / current_portfolio_value_;
    
    if (portfolio_risk > max_portfolio_risk_) {
        std::cout << "Signal rejected: exceeds portfolio risk limit" << std::endl;
        return false;
    }
    
    // Confidence threshold
    if (signal.confidence < 0.6) {
        std::cout << "Signal rejected: low confidence (" << signal.confidence << ")" << std::endl;
        return false;
    }
    
    return true;
}

void AlgorithmicEngine::executeSignal(const TradingSignal& signal) {
    // Create or update position
    Position position;
    position.symbol = signal.symbol;
    position.current_price = signal.price;
    position.entry_time = signal.timestamp;
    position.is_option = !signal.expiration_date.empty();
    
    if (position.is_option) {
        position.strike_price = signal.strike_price;
        position.expiration_date = signal.expiration_date;
        position.is_call = signal.is_call;
    }
    
    switch (signal.signal_type) {
        case SignalType::BUY:
        case SignalType::BUY_CALL:
        case SignalType::BUY_PUT:
            position.quantity = signal.quantity;
            position.average_price = signal.price;
            break;
            
        case SignalType::SELL:
        case SignalType::SELL_CALL:
        case SignalType::SELL_PUT:
            position.quantity = -signal.quantity;
            position.average_price = signal.price;
            break;
            
        case SignalType::CLOSE_POSITION:
            // Find and close existing position
            for (auto it = positions_.begin(); it != positions_.end(); ++it) {
                if (it->symbol == signal.symbol) {
                    std::cout << "Closing position: " << signal.symbol << std::endl;
                    positions_.erase(it);
                    return;
                }
            }
            return;
            
        case SignalType::HOLD:
            return; // No action needed
    }
    
    positions_.push_back(position);
    std::cout << "Position created: " << position.symbol 
              << " qty: " << position.quantity 
              << " @ " << position.average_price << std::endl;
}

void AlgorithmicEngine::updatePositions() {
    for (auto& position : positions_) {
        auto it = latest_market_data_.find(position.symbol);
        if (it != latest_market_data_.end()) {
            position.current_price = it->second.price;
            position.unrealized_pnl = (position.current_price - position.average_price) * position.quantity;
        }
    }
}

void AlgorithmicEngine::updateRiskMetrics() {
    double total_pnl = 0.0;
    for (const auto& position : positions_) {
        total_pnl += position.unrealized_pnl;
    }
    
    double portfolio_return = total_pnl / current_portfolio_value_;
    
    // Log risk metrics periodically
    static int counter = 0;
    if (++counter % 60 == 0) { // Every minute
        std::cout << "Portfolio Update - Total P&L: $" << total_pnl 
                  << ", Return: " << (portfolio_return * 100) << "%" 
                  << ", Positions: " << positions_.size() << std::endl;
    }
}

double AlgorithmicEngine::calculatePortfolioRisk() {
    double total_exposure = 0.0;
    for (const auto& position : positions_) {
        total_exposure += std::abs(position.current_price * position.quantity);
    }
    return total_exposure / current_portfolio_value_;
}

} // namespace algo
} // namespace hedgefund