#include "brownian_motion.h"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace hedgefund {
namespace options {

BrownianMotion::BrownianMotion(unsigned int seed) 
    : generator_(seed), normal_dist_(0.0, 1.0) {}

SimulationResult BrownianMotion::priceOption(const MonteCarloParams& params) {
    std::vector<double> payoffs;
    payoffs.reserve(params.num_simulations);
    
    double dt = params.time_to_expiry / params.num_steps;
    double drift = params.risk_free_rate - 0.5 * params.volatility * params.volatility;
    
    for (int sim = 0; sim < params.num_simulations; sim++) {
        double price = params.spot_price;
        
        // Simulate price path
        for (int step = 0; step < params.num_steps; step++) {
            double dW = generateNormalRandom() * std::sqrt(dt);
            price *= std::exp(drift * dt + params.volatility * dW);
        }
        
        // Calculate payoff
        double payoff = calculatePayoff(price, params.strike_price, params.is_call);
        payoffs.push_back(payoff);
    }
    
    // Calculate statistics
    double mean_payoff = std::accumulate(payoffs.begin(), payoffs.end(), 0.0) / params.num_simulations;
    double option_price = mean_payoff * std::exp(-params.risk_free_rate * params.time_to_expiry);
    
    // Calculate standard error
    double variance = 0.0;
    for (double payoff : payoffs) {
        variance += (payoff - mean_payoff) * (payoff - mean_payoff);
    }
    variance /= (params.num_simulations - 1);
    double standard_error = std::sqrt(variance / params.num_simulations) * 
                           std::exp(-params.risk_free_rate * params.time_to_expiry);
    
    // 95% confidence interval
    double z_score = 1.96; // 95% confidence
    double margin_of_error = z_score * standard_error;
    
    SimulationResult result;
    result.option_price = option_price;
    result.standard_error = standard_error;
    result.confidence_interval_lower = option_price - margin_of_error;
    result.confidence_interval_upper = option_price + margin_of_error;
    
    return result;
}

std::vector<double> BrownianMotion::generatePricePath(double spot_price, double drift, double volatility,
                                                     double time_horizon, int num_steps) {
    std::vector<double> path;
    path.reserve(num_steps + 1);
    path.push_back(spot_price);
    
    double dt = time_horizon / num_steps;
    double price = spot_price;
    
    for (int i = 0; i < num_steps; i++) {
        double dW = generateNormalRandom() * std::sqrt(dt);
        price *= std::exp((drift - 0.5 * volatility * volatility) * dt + volatility * dW);
        path.push_back(price);
    }
    
    return path;
}

std::vector<std::vector<double>> BrownianMotion::generateMultiplePaths(double spot_price, double drift, double volatility,
                                                                      double time_horizon, int num_steps, int num_paths) {
    std::vector<std::vector<double>> paths;
    paths.reserve(num_paths);
    
    for (int i = 0; i < num_paths; i++) {
        paths.push_back(generatePricePath(spot_price, drift, volatility, time_horizon, num_steps));
    }
    
    return paths;
}

double BrownianMotion::calculateVaR(const std::vector<double>& returns, double confidence_level) {
    if (returns.empty()) return 0.0;
    
    std::vector<double> sorted_returns = returns;
    std::sort(sorted_returns.begin(), sorted_returns.end());
    
    int index = static_cast<int>((1.0 - confidence_level) * sorted_returns.size());
    index = std::max(0, std::min(index, static_cast<int>(sorted_returns.size()) - 1));
    
    return -sorted_returns[index]; // VaR is typically reported as positive loss
}

std::vector<double> BrownianMotion::simulatePortfolioReturns(const std::vector<double>& weights,
                                                           const std::vector<std::vector<double>>& asset_returns,
                                                           int num_simulations) {
    std::vector<double> portfolio_returns;
    portfolio_returns.reserve(num_simulations);
    
    if (asset_returns.empty() || weights.empty()) return portfolio_returns;
    
    int num_assets = weights.size();
    int num_periods = asset_returns[0].size();
    
    for (int sim = 0; sim < num_simulations; sim++) {
        double portfolio_return = 0.0;
        
        for (int asset = 0; asset < num_assets && asset < static_cast<int>(asset_returns.size()); asset++) {
            // Random period selection
            int period = static_cast<int>(generateNormalRandom() * num_periods / 6.0 + num_periods / 2.0);
            period = std::max(0, std::min(period, num_periods - 1));
            
            if (period < static_cast<int>(asset_returns[asset].size())) {
                portfolio_return += weights[asset] * asset_returns[asset][period];
            }
        }
        
        portfolio_returns.push_back(portfolio_return);
    }
    
    return portfolio_returns;
}

double BrownianMotion::generateNormalRandom() {
    return normal_dist_(generator_);
}

double BrownianMotion::calculatePayoff(double final_price, double strike_price, bool is_call) {
    if (is_call) {
        return std::max(0.0, final_price - strike_price);
    } else {
        return std::max(0.0, strike_price - final_price);
    }
}

} // namespace options
} // namespace hedgefund