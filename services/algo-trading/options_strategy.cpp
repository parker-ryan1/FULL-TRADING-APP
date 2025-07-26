#include "options_strategy.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iostream>

namespace hedgefund {
namespace algo {

OptionsStrategy::OptionsStrategy(const StrategyConfig& config) : TradingStrategy(config) {
    std::cout << "Initialized Options Strategy: " << config.name << std::endl;
    
    // Initialize sample options chains for demonstration
    for (const auto& symbol : config.symbols) {
        OptionsChain chain;
        chain.underlying_symbol = symbol;
        chain.expiration_date = "2024-03-15"; // Sample expiration
        
        // Generate strike prices around current price (assuming $150 for demo)
        double base_price = 150.0;
        for (int i = -10; i <= 10; i++) {
            double strike = base_price + (i * 5.0); // $5 intervals
            chain.strike_prices.push_back(strike);
            
            // Sample option prices (would come from market data in real implementation)
            chain.call_prices[strike] = std::max(0.1, base_price - strike + 5.0);
            chain.put_prices[strike] = std::max(0.1, strike - base_price + 5.0);
            chain.implied_vols[strike] = 0.20 + (std::abs(strike - base_price) / base_price) * 0.1;
        }
        
        options_chains_[symbol] = chain;
    }
}

std::vector<TradingSignal> OptionsStrategy::generateSignals(const std::vector<MarketData>& market_data) {
    std::vector<TradingSignal> signals;
    
    for (const auto& data : market_data) {
        switch (config_.type) {
            case StrategyType::OPTIONS_STRADDLE: {
                auto straddle_signals = generateStraddleSignals(data);
                signals.insert(signals.end(), straddle_signals.begin(), straddle_signals.end());
                break;
            }
            
            case StrategyType::OPTIONS_STRANGLE: {
                auto strangle_signals = generateStrangleSignals(data);
                signals.insert(signals.end(), strangle_signals.begin(), strangle_signals.end());
                break;
            }
            
            case StrategyType::COVERED_CALL: {
                auto covered_call_signals = generateCoveredCallSignals(data);
                signals.insert(signals.end(), covered_call_signals.begin(), covered_call_signals.end());
                break;
            }
            
            case StrategyType::PROTECTIVE_PUT: {
                auto protective_put_signals = generateProtectivePutSignals(data);
                signals.insert(signals.end(), protective_put_signals.begin(), protective_put_signals.end());
                break;
            }
            
            case StrategyType::IRON_CONDOR: {
                auto iron_condor_signals = generateIronCondorSignals(data);
                signals.insert(signals.end(), iron_condor_signals.begin(), iron_condor_signals.end());
                break;
            }
            
            case StrategyType::BUTTERFLY_SPREAD: {
                auto butterfly_signals = generateButterflySignals(data);
                signals.insert(signals.end(), butterfly_signals.begin(), butterfly_signals.end());
                break;
            }
            
            default:
                break;
        }
    }
    
    return signals;
}

std::vector<TradingSignal> OptionsStrategy::generateStraddleSignals(const MarketData& data) {
    std::vector<TradingSignal> signals;
    
    // Long straddle: Buy call and put at same strike (ATM)
    // Best when expecting high volatility but uncertain about direction
    
    if (isLowVolatility(data.symbol)) {
        auto atm_strikes = getATMStrikes(data.symbol, data.price);
        if (!atm_strikes.empty()) {
            double atm_strike = atm_strikes[0];
            
            // Buy ATM call
            double call_price = calculateOptionPrice(data.symbol, atm_strike, true, "2024-03-15");
            signals.push_back(createOptionsSignal(data.symbol, SignalType::BUY_CALL, atm_strike, 
                true, "2024-03-15", call_price, 0.75, "Long straddle - expecting volatility increase"));
            
            // Buy ATM put
            double put_price = calculateOptionPrice(data.symbol, atm_strike, false, "2024-03-15");
            signals.push_back(createOptionsSignal(data.symbol, SignalType::BUY_PUT, atm_strike, 
                false, "2024-03-15", put_price, 0.75, "Long straddle - expecting volatility increase"));
        }
    }
    
    return signals;
}

std::vector<TradingSignal> OptionsStrategy::generateStrangleSignals(const MarketData& data) {
    std::vector<TradingSignal> signals;
    
    // Long strangle: Buy OTM call and OTM put
    // Cheaper than straddle, needs larger move to be profitable
    
    if (isLowVolatility(data.symbol)) {
        double otm_call_strike = data.price + (data.price * 0.05); // 5% OTM call
        double otm_put_strike = data.price - (data.price * 0.05);  // 5% OTM put
        
        // Buy OTM call
        double call_price = calculateOptionPrice(data.symbol, otm_call_strike, true, "2024-03-15");
        signals.push_back(createOptionsSignal(data.symbol, SignalType::BUY_CALL, otm_call_strike, 
            true, "2024-03-15", call_price, 0.70, "Long strangle - expecting large price movement"));
        
        // Buy OTM put
        double put_price = calculateOptionPrice(data.symbol, otm_put_strike, false, "2024-03-15");
        signals.push_back(createOptionsSignal(data.symbol, SignalType::BUY_PUT, otm_put_strike, 
            false, "2024-03-15", put_price, 0.70, "Long strangle - expecting large price movement"));
    }
    
    return signals;
}

std::vector<TradingSignal> OptionsStrategy::generateCoveredCallSignals(const MarketData& data) {
    std::vector<TradingSignal> signals;
    
    // Covered call: Own stock + sell call
    // Generate income from stock holdings
    
    // Check if we have stock position (simplified check)
    bool has_stock_position = false;
    for (const auto& pos : positions_) {
        if (pos.symbol == data.symbol && !pos.is_option && pos.quantity > 0) {
            has_stock_position = true;
            break;
        }
    }
    
    if (has_stock_position && data.rsi > 60) { // Slightly overbought
        double otm_call_strike = data.price + (data.price * 0.03); // 3% OTM
        double call_price = calculateOptionPrice(data.symbol, otm_call_strike, true, "2024-03-15");
        
        signals.push_back(createOptionsSignal(data.symbol, SignalType::SELL_CALL, otm_call_strike, 
            true, "2024-03-15", call_price, 0.80, "Covered call - generate income from stock position"));
    }
    
    return signals;
}

std::vector<TradingSignal> OptionsStrategy::generateProtectivePutSignals(const MarketData& data) {
    std::vector<TradingSignal> signals;
    
    // Protective put: Own stock + buy put
    // Insurance against downside
    
    bool has_stock_position = false;
    for (const auto& pos : positions_) {
        if (pos.symbol == data.symbol && !pos.is_option && pos.quantity > 0) {
            has_stock_position = true;
            break;
        }
    }
    
    if (has_stock_position && isHighVolatility(data.symbol)) {
        double otm_put_strike = data.price - (data.price * 0.05); // 5% OTM put
        double put_price = calculateOptionPrice(data.symbol, otm_put_strike, false, "2024-03-15");
        
        signals.push_back(createOptionsSignal(data.symbol, SignalType::BUY_PUT, otm_put_strike, 
            false, "2024-03-15", put_price, 0.85, "Protective put - hedge stock position"));
    }
    
    return signals;
}

std::vector<TradingSignal> OptionsStrategy::generateIronCondorSignals(const MarketData& data) {
    std::vector<TradingSignal> signals;
    
    // Iron condor: Sell ATM call/put, buy OTM call/put
    // Profit from low volatility and range-bound movement
    
    if (isHighVolatility(data.symbol) && data.rsi > 40 && data.rsi < 60) { // Neutral market
        double atm_call_strike = data.price + (data.price * 0.01);
        double atm_put_strike = data.price - (data.price * 0.01);
        double otm_call_strike = data.price + (data.price * 0.05);
        double otm_put_strike = data.price - (data.price * 0.05);
        
        // Sell ATM options
        signals.push_back(createOptionsSignal(data.symbol, SignalType::SELL_CALL, atm_call_strike, 
            true, "2024-03-15", calculateOptionPrice(data.symbol, atm_call_strike, true, "2024-03-15"), 
            0.75, "Iron condor - sell ATM call"));
        
        signals.push_back(createOptionsSignal(data.symbol, SignalType::SELL_PUT, atm_put_strike, 
            false, "2024-03-15", calculateOptionPrice(data.symbol, atm_put_strike, false, "2024-03-15"), 
            0.75, "Iron condor - sell ATM put"));
        
        // Buy OTM options for protection
        signals.push_back(createOptionsSignal(data.symbol, SignalType::BUY_CALL, otm_call_strike, 
            true, "2024-03-15", calculateOptionPrice(data.symbol, otm_call_strike, true, "2024-03-15"), 
            0.75, "Iron condor - buy OTM call protection"));
        
        signals.push_back(createOptionsSignal(data.symbol, SignalType::BUY_PUT, otm_put_strike, 
            false, "2024-03-15", calculateOptionPrice(data.symbol, otm_put_strike, false, "2024-03-15"), 
            0.75, "Iron condor - buy OTM put protection"));
    }
    
    return signals;
}

std::vector<TradingSignal> OptionsStrategy::generateButterflySignals(const MarketData& data) {
    std::vector<TradingSignal> signals;
    
    // Butterfly spread: Buy 1 ITM, sell 2 ATM, buy 1 OTM
    // Profit when stock stays near middle strike
    
    if (data.rsi > 45 && data.rsi < 55) { // Very neutral market
        double itm_strike = data.price - (data.price * 0.03);
        double atm_strike = data.price;
        double otm_strike = data.price + (data.price * 0.03);
        
        // Buy ITM call
        signals.push_back(createOptionsSignal(data.symbol, SignalType::BUY_CALL, itm_strike, 
            true, "2024-03-15", calculateOptionPrice(data.symbol, itm_strike, true, "2024-03-15"), 
            0.70, "Butterfly spread - buy ITM call"));
        
        // Sell 2 ATM calls
        signals.push_back(createOptionsSignal(data.symbol, SignalType::SELL_CALL, atm_strike, 
            true, "2024-03-15", calculateOptionPrice(data.symbol, atm_strike, true, "2024-03-15"), 
            0.70, "Butterfly spread - sell ATM calls"));
        
        // Buy OTM call
        signals.push_back(createOptionsSignal(data.symbol, SignalType::BUY_CALL, otm_strike, 
            true, "2024-03-15", calculateOptionPrice(data.symbol, otm_strike, true, "2024-03-15"), 
            0.70, "Butterfly spread - buy OTM call"));
    }
    
    return signals;
}

void OptionsStrategy::updatePosition(const Position& position) {
    bool found = false;
    for (auto& pos : positions_) {
        if (pos.symbol == position.symbol && pos.strike_price == position.strike_price && 
            pos.is_call == position.is_call && pos.expiration_date == position.expiration_date) {
            pos = position;
            found = true;
            break;
        }
    }
    
    if (!found) {
        positions_.push_back(position);
    }
}

double OptionsStrategy::calculateRisk(const std::vector<Position>& positions) {
    double total_risk = 0.0;
    
    for (const auto& position : positions) {
        if (position.is_option) {
            // Options risk is limited to premium paid for long positions
            if (position.quantity > 0) {
                total_risk += position.quantity * position.average_price;
            } else {
                // Short options have unlimited risk (simplified calculation)
                total_risk += std::abs(position.quantity) * position.current_price * 2;
            }
        } else {
            // Stock position risk
            total_risk += std::abs(position.quantity * position.current_price) * 0.2; // 20% risk assumption
        }
    }
    
    return total_risk;
}

double OptionsStrategy::calculateOptionPrice(const std::string& symbol, double strike, bool is_call, const std::string& expiration) {
    // Simplified option pricing using Black-Scholes
    hedgefund::options::OptionParams params;
    params.spot_price = 150.0; // Would get from market data
    params.strike_price = strike;
    params.time_to_expiry = getTimeToExpiration(expiration);
    params.risk_free_rate = 0.05;
    params.volatility = 0.20; // Would get implied volatility
    params.is_call = is_call;
    
    return hedgefund::options::BlackScholes::calculatePrice(params);
}

bool OptionsStrategy::isHighVolatility(const std::string& symbol) {
    auto it = options_chains_.find(symbol);
    if (it != options_chains_.end()) {
        double avg_iv = 0.0;
        for (const auto& [strike, iv] : it->second.implied_vols) {
            avg_iv += iv;
        }
        avg_iv /= it->second.implied_vols.size();
        return avg_iv > 0.25; // 25% threshold
    }
    return false;
}

bool OptionsStrategy::isLowVolatility(const std::string& symbol) {
    return !isHighVolatility(symbol);
}

double OptionsStrategy::getTimeToExpiration(const std::string& expiration_date) {
    // Simplified - assume 30 days to expiration
    return 30.0 / 365.0;
}

std::vector<double> OptionsStrategy::getATMStrikes(const std::string& symbol, double spot_price) {
    auto it = options_chains_.find(symbol);
    if (it != options_chains_.end()) {
        auto& strikes = it->second.strike_prices;
        auto closest = std::min_element(strikes.begin(), strikes.end(),
            [spot_price](double a, double b) {
                return std::abs(a - spot_price) < std::abs(b - spot_price);
            });
        
        if (closest != strikes.end()) {
            return {*closest};
        }
    }
    return {};
}

TradingSignal OptionsStrategy::createOptionsSignal(const std::string& symbol, SignalType type, double strike, 
                                                  bool is_call, const std::string& expiration, double price, 
                                                  double confidence, const std::string& reason) {
    TradingSignal signal;
    signal.strategy_id = config_.name;
    signal.symbol = symbol;
    signal.signal_type = type;
    signal.price = price;
    signal.quantity = 1; // 1 contract
    signal.confidence = confidence;
    signal.reason = reason;
    signal.timestamp = std::chrono::system_clock::now();
    signal.strike_price = strike;
    signal.expiration_date = expiration;
    signal.is_call = is_call;
    
    return signal;
}

} // namespace algo
} // namespace hedgefund