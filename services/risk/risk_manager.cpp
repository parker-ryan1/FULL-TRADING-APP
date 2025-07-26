#include "risk_manager.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <random>

namespace hedgefund {
namespace risk {

RiskManager::RiskManager() : real_time_monitoring_(false) {
    std::cout << "Risk Manager initialized" << std::endl;
    
    // Initialize default risk limits
    RiskLimit portfolio_var_limit;
    portfolio_var_limit.limit_type = "PORTFOLIO_VAR";
    portfolio_var_limit.limit_value = 0.02; // 2% daily VaR limit
    portfolio_var_limit.description = "Daily Portfolio VaR 95% confidence";
    risk_limits_.push_back(portfolio_var_limit);
    
    RiskLimit leverage_limit;
    leverage_limit.limit_type = "LEVERAGE";
    leverage_limit.limit_value = 3.0; // 3:1 leverage limit
    leverage_limit.description = "Maximum portfolio leverage";
    risk_limits_.push_back(leverage_limit);
    
    RiskLimit concentration_limit;
    concentration_limit.limit_type = "CONCENTRATION";
    concentration_limit.limit_value = 0.10; // 10% max position size
    concentration_limit.description = "Maximum single position concentration";
    risk_limits_.push_back(concentration_limit);
    
    // Initialize risk alerts
    risk_alerts_["VAR_BREACH"] = 0.015; // Alert at 1.5% VaR
    risk_alerts_["DRAWDOWN"] = 0.05; // Alert at 5% drawdown
    risk_alerts_["LEVERAGE"] = 2.5; // Alert at 2.5x leverage
    
    loadHistoricalData();
}

RiskManager::~RiskManager() {
    stopRealTimeMonitoring();
}

RiskMetrics RiskManager::calculatePortfolioRisk(const std::vector<Position>& positions) {
    RiskMetrics metrics;
    metrics.calculation_time = std::chrono::system_clock::now();
    
    // Calculate basic portfolio metrics
    metrics.portfolio_value = 0.0;
    metrics.total_exposure = 0.0;
    metrics.net_exposure = 0.0;
    metrics.gross_exposure = 0.0;
    
    for (const auto& position : positions) {
        metrics.portfolio_value += position.market_value;
        metrics.gross_exposure += std::abs(position.market_value);
        metrics.net_exposure += position.market_value;
    }
    
    metrics.leverage = metrics.portfolio_value != 0 ? metrics.gross_exposure / metrics.portfolio_value : 0.0;
    
    // Calculate VaR using different methods
    metrics.var_1day_95 = calculateParametricVaR(positions, 0.95);
    metrics.var_1day_99 = calculateParametricVaR(positions, 0.99);
    
    // Calculate Expected Shortfall
    std::vector<double> simulated_returns = generateRandomReturns(positions);
    metrics.cvar_1day_95 = calculateExpectedShortfall(simulated_returns, 0.95);
    metrics.expected_shortfall = metrics.cvar_1day_95;
    
    // Calculate portfolio volatility
    double portfolio_variance = 0.0;
    for (size_t i = 0; i < positions.size(); ++i) {
        for (size_t j = 0; j < positions.size(); ++j) {
            double weight_i = positions[i].market_value / metrics.portfolio_value;
            double weight_j = positions[j].market_value / metrics.portfolio_value;
            double vol_i = 0.20; // Simplified - would use historical volatility
            double vol_j = 0.20;
            double correlation = i == j ? 1.0 : 0.3; // Simplified correlation
            
            portfolio_variance += weight_i * weight_j * vol_i * vol_j * correlation;
        }
    }
    metrics.volatility = std::sqrt(portfolio_variance);
    
    // Calculate Sharpe ratio (simplified)
    double risk_free_rate = 0.02;
    double expected_return = 0.08; // Simplified
    metrics.sharpe_ratio = (expected_return - risk_free_rate) / metrics.volatility;
    
    // Calculate beta (simplified)
    metrics.beta = 1.0; // Would calculate against benchmark
    metrics.correlation_spy = 0.7; // Simplified
    
    std::cout << "Portfolio Risk Calculated - VaR 95%: " << (metrics.var_1day_95 * 100) 
              << "%, Leverage: " << metrics.leverage << "x" << std::endl;
    
    return metrics;
}

double RiskManager::calculateHistoricalVaR(const std::vector<double>& returns, double confidence) {
    if (returns.empty()) return 0.0;
    
    std::vector<double> sorted_returns = returns;
    std::sort(sorted_returns.begin(), sorted_returns.end());
    
    int index = static_cast<int>((1.0 - confidence) * sorted_returns.size());
    index = std::max(0, std::min(index, static_cast<int>(sorted_returns.size()) - 1));
    
    return -sorted_returns[index]; // VaR is reported as positive loss
}

double RiskManager::calculateParametricVaR(const std::vector<Position>& positions, double confidence) {
    if (positions.empty()) return 0.0;
    
    // Calculate portfolio standard deviation
    double portfolio_value = 0.0;
    for (const auto& pos : positions) {
        portfolio_value += pos.market_value;
    }
    
    if (portfolio_value == 0) return 0.0;
    
    // Simplified parametric VaR calculation
    double portfolio_volatility = 0.20; // 20% annual volatility assumption
    double daily_volatility = portfolio_volatility / std::sqrt(252); // Convert to daily
    
    // Z-score for confidence level
    double z_score = confidence == 0.95 ? 1.645 : (confidence == 0.99 ? 2.326 : 1.645);
    
    double var = z_score * daily_volatility * portfolio_value;
    return var / portfolio_value; // Return as percentage
}

double RiskManager::calculateMonteCarloVaR(const std::vector<Position>& positions, int simulations, double confidence) {
    if (positions.empty()) return 0.0;
    
    std::vector<double> simulated_pnl;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> normal_dist(0.0, 1.0);
    
    double portfolio_value = 0.0;
    for (const auto& pos : positions) {
        portfolio_value += pos.market_value;
    }
    
    for (int i = 0; i < simulations; ++i) {
        double total_pnl = 0.0;
        
        for (const auto& position : positions) {
            // Generate random return for each position
            double volatility = 0.20 / std::sqrt(252); // Daily volatility
            double random_return = normal_dist(gen) * volatility;
            double position_pnl = position.market_value * random_return;
            total_pnl += position_pnl;
        }
        
        simulated_pnl.push_back(total_pnl);
    }
    
    return calculateHistoricalVaR(simulated_pnl, confidence) / portfolio_value;
}

double RiskManager::calculateExpectedShortfall(const std::vector<double>& returns, double confidence) {
    if (returns.empty()) return 0.0;
    
    std::vector<double> sorted_returns = returns;
    std::sort(sorted_returns.begin(), sorted_returns.end());
    
    int var_index = static_cast<int>((1.0 - confidence) * sorted_returns.size());
    var_index = std::max(0, std::min(var_index, static_cast<int>(sorted_returns.size()) - 1));
    
    double sum = 0.0;
    for (int i = 0; i <= var_index; ++i) {
        sum += sorted_returns[i];
    }
    
    return -(sum / (var_index + 1)); // Expected Shortfall as positive value
}

std::vector<StressTestResult> RiskManager::runStressTests(const std::vector<Position>& positions) {
    std::vector<StressTestResult> results;
    
    auto scenarios = getStandardStressScenarios();
    
    for (const auto& scenario : scenarios) {
        StressTestResult result = runCustomStressTest(positions, scenario);
        results.push_back(result);
    }
    
    std::cout << "Completed " << results.size() << " stress test scenarios" << std::endl;
    return results;
}

StressTestResult RiskManager::runCustomStressTest(const std::vector<Position>& positions, const StressTestScenario& scenario) {
    StressTestResult result;
    result.scenario_name = scenario.name;
    result.portfolio_pnl = 0.0;
    result.worst_position_loss = 0.0;
    
    double total_portfolio_value = 0.0;
    for (const auto& pos : positions) {
        total_portfolio_value += pos.market_value;
    }
    
    for (const auto& position : positions) {
        double position_pnl = 0.0;
        
        // Apply price shock if exists for this symbol
        auto shock_it = scenario.price_shocks.find(position.symbol);
        if (shock_it != scenario.price_shocks.end()) {
            position_pnl = position.market_value * shock_it->second;
        } else {
            // Apply general market shock (simplified)
            position_pnl = position.market_value * -0.05; // -5% default shock
        }
        
        // Apply Greeks effects for options
        if (position.is_option) {
            // Volatility shock effect
            position_pnl += position.vega * scenario.volatility_shock;
            
            // Interest rate shock effect
            position_pnl += position.rho * scenario.interest_rate_shock;
        }
        
        result.position_pnl[position.symbol] = position_pnl;
        result.portfolio_pnl += position_pnl;
        
        if (position_pnl < result.worst_position_loss) {
            result.worst_position_loss = position_pnl;
            result.worst_position_symbol = position.symbol;
        }
    }
    
    result.portfolio_return = total_portfolio_value != 0 ? result.portfolio_pnl / total_portfolio_value : 0.0;
    
    // Check for breached limits
    result.breached_limits = checkRiskLimits(positions);
    
    std::cout << "Stress test '" << scenario.name << "': Portfolio P&L = $" 
              << result.portfolio_pnl << " (" << (result.portfolio_return * 100) << "%)" << std::endl;
    
    return result;
}

std::vector<RiskLimit> RiskManager::checkRiskLimits(const std::vector<Position>& positions) {
    std::vector<RiskLimit> breached_limits;
    
    RiskMetrics metrics = calculatePortfolioRisk(positions);
    
    for (auto& limit : risk_limits_) {
        limit.is_breached = false;
        
        if (limit.limit_type == "PORTFOLIO_VAR") {
            limit.current_value = metrics.var_1day_95;
            limit.utilization_pct = (limit.current_value / limit.limit_value) * 100;
            limit.is_breached = limit.current_value > limit.limit_value;
        }
        else if (limit.limit_type == "LEVERAGE") {
            limit.current_value = metrics.leverage;
            limit.utilization_pct = (limit.current_value / limit.limit_value) * 100;
            limit.is_breached = limit.current_value > limit.limit_value;
        }
        else if (limit.limit_type == "CONCENTRATION") {
            // Check largest position concentration
            double max_concentration = 0.0;
            for (const auto& pos : positions) {
                double concentration = std::abs(pos.market_value) / metrics.portfolio_value;
                max_concentration = std::max(max_concentration, concentration);
            }
            limit.current_value = max_concentration;
            limit.utilization_pct = (limit.current_value / limit.limit_value) * 100;
            limit.is_breached = limit.current_value > limit.limit_value;
        }
        
        if (limit.is_breached) {
            breached_limits.push_back(limit);
        }
    }
    
    return breached_limits;
}

double RiskManager::calculatePortfolioDelta(const std::vector<Position>& positions) {
    double total_delta = 0.0;
    for (const auto& pos : positions) {
        if (pos.is_option) {
            total_delta += pos.delta * pos.quantity;
        } else {
            total_delta += pos.quantity; // Stock delta = 1
        }
    }
    return total_delta;
}

double RiskManager::calculatePortfolioGamma(const std::vector<Position>& positions) {
    double total_gamma = 0.0;
    for (const auto& pos : positions) {
        if (pos.is_option) {
            total_gamma += pos.gamma * pos.quantity;
        }
    }
    return total_gamma;
}

double RiskManager::calculatePortfolioTheta(const std::vector<Position>& positions) {
    double total_theta = 0.0;
    for (const auto& pos : positions) {
        if (pos.is_option) {
            total_theta += pos.theta * pos.quantity;
        }
    }
    return total_theta;
}

double RiskManager::calculatePortfolioVega(const std::vector<Position>& positions) {
    double total_vega = 0.0;
    for (const auto& pos : positions) {
        if (pos.is_option) {
            total_vega += pos.vega * pos.quantity;
        }
    }
    return total_vega;
}

std::vector<std::string> RiskManager::checkRiskAlerts(const RiskMetrics& metrics) {
    std::vector<std::string> alerts;
    
    if (metrics.var_1day_95 > risk_alerts_["VAR_BREACH"]) {
        alerts.push_back("VaR breach detected: " + std::to_string(metrics.var_1day_95 * 100) + "%");
    }
    
    if (metrics.leverage > risk_alerts_["LEVERAGE"]) {
        alerts.push_back("High leverage detected: " + std::to_string(metrics.leverage) + "x");
    }
    
    if (metrics.maximum_drawdown > risk_alerts_["DRAWDOWN"]) {
        alerts.push_back("Drawdown alert: " + std::to_string(metrics.maximum_drawdown * 100) + "%");
    }
    
    return alerts;
}

void RiskManager::loadHistoricalData() {
    // Load simplified historical data for risk calculations
    std::vector<std::string> symbols = {"AAPL", "GOOGL", "TSLA", "MSFT", "AMZN"};
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> return_dist(0.0008, 0.02); // Daily returns
    
    for (const auto& symbol : symbols) {
        std::vector<double> returns;
        for (int i = 0; i < 252; ++i) { // 1 year of daily returns
            returns.push_back(return_dist(gen));
        }
        historical_returns_[symbol] = returns;
    }
    
    std::cout << "Loaded historical data for " << symbols.size() << " symbols" << std::endl;
}

std::vector<StressTestScenario> RiskManager::getStandardStressScenarios() {
    std::vector<StressTestScenario> scenarios;
    
    scenarios.push_back(createMarketCrashScenario());
    scenarios.push_back(createInterestRateShockScenario());
    scenarios.push_back(createVolatilityShockScenario());
    scenarios.push_back(createSectorRotationScenario());
    
    return scenarios;
}

StressTestScenario RiskManager::createMarketCrashScenario() {
    StressTestScenario scenario;
    scenario.name = "Market Crash 2008 Style";
    scenario.description = "Severe market downturn similar to 2008 financial crisis";
    
    // Apply severe negative shocks to all major symbols
    scenario.price_shocks["AAPL"] = -0.30;   // -30%
    scenario.price_shocks["GOOGL"] = -0.35;  // -35%
    scenario.price_shocks["TSLA"] = -0.45;   // -45%
    scenario.price_shocks["MSFT"] = -0.25;   // -25%
    scenario.price_shocks["AMZN"] = -0.40;   // -40%
    scenario.price_shocks["SPY"] = -0.30;    // -30%
    
    scenario.interest_rate_shock = -0.02;    // -200 bps
    scenario.volatility_shock = 0.15;        // +15% volatility
    scenario.correlation_shock = 0.3;        // Correlations increase to 0.9
    
    return scenario;
}

StressTestScenario RiskManager::createInterestRateShockScenario() {
    StressTestScenario scenario;
    scenario.name = "Interest Rate Shock";
    scenario.description = "Sudden 300 basis point increase in interest rates";
    
    // Moderate price shocks, focus on rate-sensitive sectors
    scenario.price_shocks["AAPL"] = -0.10;
    scenario.price_shocks["GOOGL"] = -0.08;
    scenario.price_shocks["TSLA"] = -0.15;
    scenario.price_shocks["MSFT"] = -0.12;
    
    scenario.interest_rate_shock = 0.03;     // +300 bps
    scenario.volatility_shock = 0.05;        // +5% volatility
    scenario.correlation_shock = 0.1;
    
    return scenario;
}

StressTestScenario RiskManager::createVolatilityShockScenario() {
    StressTestScenario scenario;
    scenario.name = "Volatility Spike";
    scenario.description = "Sudden spike in market volatility (VIX to 50+)";
    
    // Moderate price movements but high volatility
    scenario.price_shocks["AAPL"] = -0.05;
    scenario.price_shocks["GOOGL"] = -0.08;
    scenario.price_shocks["TSLA"] = -0.12;
    
    scenario.interest_rate_shock = 0.0;
    scenario.volatility_shock = 0.25;        // +25% volatility shock
    scenario.correlation_shock = 0.2;
    
    return scenario;
}

StressTestScenario RiskManager::createSectorRotationScenario() {
    StressTestScenario scenario;
    scenario.name = "Tech Sector Rotation";
    scenario.description = "Rotation out of technology stocks into value";
    
    // Tech stocks down, others neutral
    scenario.price_shocks["AAPL"] = -0.20;
    scenario.price_shocks["GOOGL"] = -0.25;
    scenario.price_shocks["MSFT"] = -0.18;
    scenario.price_shocks["TSLA"] = -0.30;
    scenario.price_shocks["AMZN"] = -0.22;
    
    scenario.interest_rate_shock = 0.01;
    scenario.volatility_shock = 0.08;
    scenario.correlation_shock = -0.1;       // Correlations decrease
    
    return scenario;
}

std::vector<double> RiskManager::generateRandomReturns(const std::vector<Position>& positions) {
    std::vector<double> returns;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> normal_dist(0.0, 0.02);
    
    for (int i = 0; i < 1000; ++i) {
        double portfolio_return = 0.0;
        double total_value = 0.0;
        
        for (const auto& pos : positions) {
            total_value += pos.market_value;
        }
        
        for (const auto& pos : positions) {
            double weight = pos.market_value / total_value;
            double asset_return = normal_dist(gen);
            portfolio_return += weight * asset_return;
        }
        
        returns.push_back(portfolio_return);
    }
    
    return returns;
}

void RiskManager::startRealTimeMonitoring() {
    real_time_monitoring_ = true;
    std::cout << "Real-time risk monitoring started" << std::endl;
}

void RiskManager::stopRealTimeMonitoring() {
    real_time_monitoring_ = false;
    std::cout << "Real-time risk monitoring stopped" << std::endl;
}

std::vector<RiskLimit> RiskManager::getBreachedLimits(const std::vector<Position>& positions) {
    return checkRiskLimits(positions);
}

} // namespace risk
} // namespace hedgefund