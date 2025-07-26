#pragma once

#include <vector>
#include <random>

namespace hedgefund {
namespace options {

struct MonteCarloParams {
    double spot_price;
    double strike_price;
    double time_to_expiry;
    double risk_free_rate;
    double volatility;
    bool is_call;
    int num_simulations;
    int num_steps;
};

struct SimulationResult {
    double option_price;
    double standard_error;
    std::vector<double> price_paths;
    double confidence_interval_lower;
    double confidence_interval_upper;
};

class BrownianMotion {
public:
    BrownianMotion(unsigned int seed = std::random_device{}());
    
    // Monte Carlo option pricing
    SimulationResult priceOption(const MonteCarloParams& params);
    
    // Generate single price path using Geometric Brownian Motion
    std::vector<double> generatePricePath(double spot_price, double drift, double volatility, 
                                         double time_horizon, int num_steps);
    
    // Generate multiple price paths
    std::vector<std::vector<double>> generateMultiplePaths(double spot_price, double drift, double volatility,
                                                          double time_horizon, int num_steps, int num_paths);
    
    // Calculate Value at Risk using Monte Carlo
    double calculateVaR(const std::vector<double>& returns, double confidence_level);
    
    // Simulate portfolio returns
    std::vector<double> simulatePortfolioReturns(const std::vector<double>& weights,
                                               const std::vector<std::vector<double>>& asset_returns,
                                               int num_simulations);

private:
    std::mt19937 generator_;
    std::normal_distribution<double> normal_dist_;
    
    double generateNormalRandom();
    double calculatePayoff(double final_price, double strike_price, bool is_call);
};

} // namespace options
} // namespace hedgefund