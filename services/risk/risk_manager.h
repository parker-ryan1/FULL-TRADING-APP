#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>

namespace hedgefund {
namespace risk {

struct Position {
    std::string symbol;
    double quantity;
    double market_value;
    double unrealized_pnl;
    double delta; // For options
    double gamma;
    double theta;
    double vega;
    double rho;
    bool is_option;
    std::string expiration_date;
    double strike_price;
};

struct RiskMetrics {
    double portfolio_value;
    double total_exposure;
    double net_exposure;
    double gross_exposure;
    double leverage;
    double var_1day_95;
    double var_1day_99;
    double cvar_1day_95;
    double expected_shortfall;
    double maximum_drawdown;
    double beta;
    double correlation_spy;
    double volatility;
    double sharpe_ratio;
    std::chrono::system_clock::time_point calculation_time;
};

struct RiskLimit {
    std::string limit_type;
    std::string symbol; // Empty for portfolio-wide limits
    double limit_value;
    double current_value;
    double utilization_pct;
    bool is_breached;
    std::string description;
};

struct StressTestScenario {
    std::string name;
    std::string description;
    std::unordered_map<std::string, double> price_shocks; // symbol -> shock percentage
    double interest_rate_shock;
    double volatility_shock;
    double correlation_shock;
};

struct StressTestResult {
    std::string scenario_name;
    double portfolio_pnl;
    double portfolio_return;
    std::unordered_map<std::string, double> position_pnl;
    std::vector<RiskLimit> breached_limits;
    double worst_position_loss;
    std::string worst_position_symbol;
};

class RiskManager {
public:
    RiskManager();
    ~RiskManager();
    
    // Portfolio risk calculation
    RiskMetrics calculatePortfolioRisk(const std::vector<Position>& positions);
    
    // VaR calculations
    double calculateHistoricalVaR(const std::vector<double>& returns, double confidence = 0.95);
    double calculateParametricVaR(const std::vector<Position>& positions, double confidence = 0.95);
    double calculateMonteCarloVaR(const std::vector<Position>& positions, int simulations = 10000, double confidence = 0.95);
    
    // Expected Shortfall (Conditional VaR)
    double calculateExpectedShortfall(const std::vector<double>& returns, double confidence = 0.95);
    
    // Stress testing
    std::vector<StressTestResult> runStressTests(const std::vector<Position>& positions);
    StressTestResult runCustomStressTest(const std::vector<Position>& positions, const StressTestScenario& scenario);
    
    // Risk limits management
    void setRiskLimit(const RiskLimit& limit);
    std::vector<RiskLimit> checkRiskLimits(const std::vector<Position>& positions);
    std::vector<RiskLimit> getBreachedLimits(const std::vector<Position>& positions);
    
    // Greeks aggregation for options portfolios
    double calculatePortfolioDelta(const std::vector<Position>& positions);
    double calculatePortfolioGamma(const std::vector<Position>& positions);
    double calculatePortfolioTheta(const std::vector<Position>& positions);
    double calculatePortfolioVega(const std::vector<Position>& positions);
    
    // Correlation and diversification
    double calculatePortfolioCorrelation(const std::vector<Position>& positions);
    double calculateDiversificationRatio(const std::vector<Position>& positions);
    
    // Risk attribution
    std::unordered_map<std::string, double> calculateRiskContribution(const std::vector<Position>& positions);
    std::unordered_map<std::string, double> calculateVaRContribution(const std::vector<Position>& positions);
    
    // Liquidity risk
    double calculateLiquidityRisk(const std::vector<Position>& positions);
    std::vector<std::string> identifyIlliquidPositions(const std::vector<Position>& positions);
    
    // Real-time monitoring
    void startRealTimeMonitoring();
    void stopRealTimeMonitoring();
    bool isRealTimeMonitoringActive();
    
    // Alerts and notifications
    void setRiskAlert(const std::string& alert_type, double threshold);
    std::vector<std::string> checkRiskAlerts(const RiskMetrics& metrics);
    
private:
    std::vector<RiskLimit> risk_limits_;
    std::unordered_map<std::string, double> risk_alerts_;
    bool real_time_monitoring_;
    
    // Historical data for calculations
    std::unordered_map<std::string, std::vector<double>> historical_returns_;
    std::unordered_map<std::string, std::vector<double>> historical_prices_;
    
    // Correlation matrix
    std::unordered_map<std::string, std::unordered_map<std::string, double>> correlation_matrix_;
    
    // Helper methods
    void loadHistoricalData();
    void updateCorrelationMatrix();
    std::vector<double> generateRandomReturns(const std::vector<Position>& positions);
    double calculatePortfolioReturn(const std::vector<Position>& positions, const std::vector<double>& returns);
    
    // Predefined stress test scenarios
    std::vector<StressTestScenario> getStandardStressScenarios();
    StressTestScenario createMarketCrashScenario();
    StressTestScenario createInterestRateShockScenario();
    StressTestScenario createVolatilityShockScenario();
    StressTestScenario createSectorRotationScenario();
};

} // namespace risk
} // namespace hedgefund