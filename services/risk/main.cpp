#include "risk_manager.h"
#include "common/database.h"
#include "common/messaging.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <random>

using namespace hedgefund::risk;
using namespace hedgefund::common;

class RiskService {
public:
    RiskService() 
        : db_("host=localhost port=5432 dbname=hedgefund user=trader password=secure_password"),
          mq_("tcp://localhost:61616"),
          risk_manager_() {}
    
    bool initialize() {
        if (!db_.connect()) {
            std::cerr << "Failed to connect to database" << std::endl;
            return false;
        }
        
        if (!mq_.connect()) {
            std::cerr << "Failed to connect to message queue" << std::endl;
            return false;
        }
        
        // Subscribe to risk calculation requests
        mq_.subscribe("risk.calculate", [this](const Message& msg) {
            handleRiskCalculationRequest(msg);
        });
        
        mq_.subscribe("risk.stress_test", [this](const Message& msg) {
            handleStressTestRequest(msg);
        });
        
        mq_.subscribe("portfolio.update", [this](const Message& msg) {
            handlePortfolioUpdate(msg);
        });
        
        mq_.startConsumer();
        
        std::cout << "Risk Service initialized" << std::endl;
        return true;
    }
    
    void run() {
        std::cout << "Risk Service started" << std::endl;
        
        // Start real-time risk monitoring
        risk_manager_.startRealTimeMonitoring();
        
        while (true) {
            // Periodic risk calculations
            calculatePortfolioRisk();
            
            // Run stress tests periodically
            runPeriodicStressTests();
            
            // Check risk limits and alerts
            checkRiskLimitsAndAlerts();
            
            std::this_thread::sleep_for(std::chrono::seconds(30));
        }
    }
    
private:
    Database db_;
    MessageQueue mq_;
    RiskManager risk_manager_;
    std::vector<Position> current_positions_;
    
    void calculatePortfolioRisk() {
        if (current_positions_.empty()) {
            // Load positions from database or create sample positions
            loadCurrentPositions();
        }
        
        RiskMetrics metrics = risk_manager_.calculatePortfolioRisk(current_positions_);
        
        // Store risk metrics in database
        storeRiskMetrics(metrics);
        
        // Publish risk metrics
        publishRiskMetrics(metrics);
        
        // Check for alerts
        auto alerts = risk_manager_.checkRiskAlerts(metrics);
        for (const auto& alert : alerts) {
            publishRiskAlert(alert);
        }
    }
    
    void runPeriodicStressTests() {
        static int stress_test_counter = 0;
        
        // Run stress tests every 5 minutes (10 cycles)
        if (++stress_test_counter % 10 == 0) {
            std::cout << "Running periodic stress tests..." << std::endl;
            
            auto stress_results = risk_manager_.runStressTests(current_positions_);
            
            for (const auto& result : stress_results) {
                publishStressTestResult(result);
                
                // Store in database
                storeStressTestResult(result);
            }
        }
    }
    
    void checkRiskLimitsAndAlerts() {
        auto breached_limits = risk_manager_.getBreachedLimits(current_positions_);
        
        for (const auto& limit : breached_limits) {
            publishRiskLimitBreach(limit);
            std::cout << "RISK LIMIT BREACH: " << limit.description 
                      << " - Current: " << limit.current_value 
                      << ", Limit: " << limit.limit_value << std::endl;
        }
    }
    
    void loadCurrentPositions() {
        // Load positions from database or create sample positions for demonstration
        current_positions_.clear();
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> value_dist(10000, 100000);
        std::uniform_real_distribution<> greek_dist(-1.0, 1.0);
        
        std::vector<std::string> symbols = {"AAPL", "GOOGL", "TSLA", "MSFT", "AMZN", "NVDA"};
        
        for (const auto& symbol : symbols) {
            // Stock position
            Position stock_pos;
            stock_pos.symbol = symbol;
            stock_pos.quantity = 100 + (gen() % 500);
            stock_pos.market_value = value_dist(gen);
            stock_pos.unrealized_pnl = (gen() % 10000) - 5000;
            stock_pos.is_option = false;
            current_positions_.push_back(stock_pos);
            
            // Options position
            Position option_pos;
            option_pos.symbol = symbol + "_CALL_150";
            option_pos.quantity = 10 + (gen() % 20);
            option_pos.market_value = value_dist(gen) * 0.1; // Smaller options positions
            option_pos.unrealized_pnl = (gen() % 5000) - 2500;
            option_pos.is_option = true;
            option_pos.delta = greek_dist(gen);
            option_pos.gamma = std::abs(greek_dist(gen)) * 0.1;
            option_pos.theta = -std::abs(greek_dist(gen)) * 0.05;
            option_pos.vega = std::abs(greek_dist(gen)) * 0.2;
            option_pos.rho = greek_dist(gen) * 0.1;
            option_pos.strike_price = 150.0;
            option_pos.expiration_date = "2024-03-15";
            current_positions_.push_back(option_pos);
        }
        
        std::cout << "Loaded " << current_positions_.size() << " positions for risk calculation" << std::endl;
    }
    
    void handleRiskCalculationRequest(const Message& msg) {
        std::cout << "Received risk calculation request: " << msg.payload << std::endl;
        
        // Parse request and calculate risk for specific portfolio
        RiskMetrics metrics = risk_manager_.calculatePortfolioRisk(current_positions_);
        publishRiskMetrics(metrics);
    }
    
    void handleStressTestRequest(const Message& msg) {
        std::cout << "Received stress test request: " << msg.payload << std::endl;
        
        auto stress_results = risk_manager_.runStressTests(current_positions_);
        
        for (const auto& result : stress_results) {
            publishStressTestResult(result);
        }
    }
    
    void handlePortfolioUpdate(const Message& msg) {
        std::cout << "Received portfolio update: " << msg.payload << std::endl;
        
        // Update current positions and recalculate risk
        loadCurrentPositions(); // In production, would parse the update
        calculatePortfolioRisk();
    }
    
    void storeRiskMetrics(const RiskMetrics& metrics) {
        std::ostringstream query;
        query << "INSERT INTO risk_metrics (client_id, metric_type, metric_value, timestamp) VALUES "
              << "('PORTFOLIO', 'VAR_95', " << metrics.var_1day_95 << ", NOW()), "
              << "('PORTFOLIO', 'VAR_99', " << metrics.var_1day_99 << ", NOW()), "
              << "('PORTFOLIO', 'LEVERAGE', " << metrics.leverage << ", NOW()), "
              << "('PORTFOLIO', 'VOLATILITY', " << metrics.volatility << ", NOW()), "
              << "('PORTFOLIO', 'SHARPE_RATIO', " << metrics.sharpe_ratio << ", NOW())";
        
        db_.execute(query.str());
    }
    
    void storeStressTestResult(const StressTestResult& result) {
        std::ostringstream query;
        query << "INSERT INTO risk_metrics (client_id, metric_type, metric_value, timestamp) VALUES "
              << "('" << result.scenario_name << "', 'STRESS_TEST_PNL', " << result.portfolio_pnl << ", NOW())";
        
        db_.execute(query.str());
    }
    
    void publishRiskMetrics(const RiskMetrics& metrics) {
        std::ostringstream msg;
        msg << "RISK_METRICS,"
            << metrics.portfolio_value << ","
            << metrics.var_1day_95 << ","
            << metrics.var_1day_99 << ","
            << metrics.leverage << ","
            << metrics.volatility << ","
            << metrics.sharpe_ratio << ","
            << metrics.beta;
        
        mq_.publish("risk.metrics", msg.str());
        
        // Publish Greeks for options portfolios
        double portfolio_delta = risk_manager_.calculatePortfolioDelta(current_positions_);
        double portfolio_gamma = risk_manager_.calculatePortfolioGamma(current_positions_);
        double portfolio_theta = risk_manager_.calculatePortfolioTheta(current_positions_);
        double portfolio_vega = risk_manager_.calculatePortfolioVega(current_positions_);
        
        std::ostringstream greeks_msg;
        greeks_msg << "PORTFOLIO_GREEKS,"
                   << portfolio_delta << ","
                   << portfolio_gamma << ","
                   << portfolio_theta << ","
                   << portfolio_vega;
        
        mq_.publish("risk.greeks", greeks_msg.str());
        
        std::cout << "Published risk metrics - VaR 95%: " << (metrics.var_1day_95 * 100) 
                  << "%, Leverage: " << metrics.leverage << "x" << std::endl;
    }
    
    void publishStressTestResult(const StressTestResult& result) {
        std::ostringstream msg;
        msg << "STRESS_TEST_RESULT,"
            << result.scenario_name << ","
            << result.portfolio_pnl << ","
            << result.portfolio_return << ","
            << result.worst_position_symbol << ","
            << result.worst_position_loss;
        
        mq_.publish("risk.stress_test", msg.str());
        
        std::cout << "Stress test '" << result.scenario_name 
                  << "': Portfolio P&L = $" << result.portfolio_pnl << std::endl;
    }
    
    void publishRiskAlert(const std::string& alert) {
        std::ostringstream msg;
        msg << "RISK_ALERT," << alert;
        
        mq_.publish("risk.alert", msg.str());
        
        std::cout << "RISK ALERT: " << alert << std::endl;
    }
    
    void publishRiskLimitBreach(const RiskLimit& limit) {
        std::ostringstream msg;
        msg << "RISK_LIMIT_BREACH,"
            << limit.limit_type << ","
            << limit.current_value << ","
            << limit.limit_value << ","
            << limit.utilization_pct << ","
            << limit.description;
        
        mq_.publish("risk.limit_breach", msg.str());
    }
};

int main() {
    std::cout << "Starting Risk Management Service..." << std::endl;
    
    RiskService service;
    
    if (!service.initialize()) {
        std::cerr << "Failed to initialize Risk Service" << std::endl;
        return 1;
    }
    
    service.run();
    return 0;
}