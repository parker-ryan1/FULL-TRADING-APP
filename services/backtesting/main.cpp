#include "backtesting_engine.h"
#include "../algo-trading/momentum_strategy.h"
#include "../algo-trading/options_strategy.h"
#include "common/database.h"
#include "common/messaging.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>

using namespace hedgefund::backtesting;
using namespace hedgefund::algo;
using namespace hedgefund::common;

class BacktestingService {
public:
    BacktestingService() 
        : db_("host=localhost port=5432 dbname=hedgefund user=trader password=secure_password"),
          mq_("tcp://localhost:61616"),
          engine_() {}
    
    bool initialize() {
        if (!db_.connect()) {
            std::cerr << "Failed to connect to database" << std::endl;
            return false;
        }
        
        if (!mq_.connect()) {
            std::cerr << "Failed to connect to message queue" << std::endl;
            return false;
        }
        
        // Subscribe to backtesting requests
        mq_.subscribe("backtest.request", [this](const Message& msg) {
            handleBacktestRequest(msg);
        });
        
        mq_.subscribe("backtest.optimize", [this](const Message& msg) {
            handleOptimizationRequest(msg);
        });
        
        mq_.startConsumer();
        
        std::cout << "Backtesting Service initialized" << std::endl;
        return true;
    }
    
    void run() {
        std::cout << "Backtesting Service started" << std::endl;
        
        // Run demonstration backtests
        runDemonstrationBacktests();
        
        // Keep service running
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }
    
private:
    Database db_;
    MessageQueue mq_;
    BacktestingEngine engine_;
    
    void runDemonstrationBacktests() {
        std::cout << "\n=== Running Demonstration Backtests ===" << std::endl;
        
        // Test Momentum Strategy
        runMomentumStrategyBacktest();
        
        // Test Options Strategy
        runOptionsStrategyBacktest();
        
        // Run comparison analysis
        runStrategyComparison();
    }
    
    void runMomentumStrategyBacktest() {
        std::cout << "\n--- Momentum Strategy Backtest ---" << std::endl;
        
        BacktestConfig config;
        config.strategy_name = "Momentum_Strategy_Backtest";
        config.start_date = std::chrono::system_clock::now() - std::chrono::hours(24 * 365); // 1 year ago
        config.end_date = std::chrono::system_clock::now();
        config.symbols = {"AAPL", "GOOGL", "TSLA"};
        config.starting_capital = 1000000.0; // $1M
        config.commission_per_trade = 5.0;
        config.slippage_bps = 5.0; // 5 basis points
        config.allow_short_selling = true;
        config.max_position_size = 0.1; // 10% max position
        config.benchmark_symbol = "SPY";
        
        // Strategy parameters
        config.parameters["momentum_threshold"] = 0.02;
        config.parameters["volatility_threshold"] = 0.03;
        config.parameters["rsi_overbought"] = 70.0;
        config.parameters["rsi_oversold"] = 30.0;
        
        // Create strategy
        StrategyConfig strategy_config;
        strategy_config.type = StrategyType::MOMENTUM;
        strategy_config.name = "Momentum_Backtest";
        strategy_config.parameters = config.parameters;
        strategy_config.symbols = config.symbols;
        strategy_config.enabled = true;
        strategy_config.max_position_size = 100.0;
        
        auto strategy = std::make_unique<MomentumStrategy>(strategy_config);
        
        // Run backtest
        BacktestResults results = engine_.runBacktest(config, std::move(strategy));
        
        // Generate report
        engine_.generatePerformanceReport(results, "momentum_strategy_backtest.txt");
        
        // Publish results
        publishBacktestResults(results);
    }
    
    void runOptionsStrategyBacktest() {
        std::cout << "\n--- Options Strategy Backtest ---" << std::endl;
        
        BacktestConfig config;
        config.strategy_name = "Options_Straddle_Backtest";
        config.start_date = std::chrono::system_clock::now() - std::chrono::hours(24 * 180); // 6 months ago
        config.end_date = std::chrono::system_clock::now();
        config.symbols = {"AAPL", "GOOGL"};
        config.starting_capital = 500000.0; // $500K
        config.commission_per_trade = 1.0; // Lower commission for options
        config.slippage_bps = 10.0; // Higher slippage for options
        config.allow_short_selling = true;
        config.max_position_size = 0.05; // 5% max position for options
        
        // Create options strategy
        StrategyConfig strategy_config;
        strategy_config.type = StrategyType::OPTIONS_STRADDLE;
        strategy_config.name = "Options_Straddle_Backtest";
        strategy_config.symbols = config.symbols;
        strategy_config.enabled = true;
        strategy_config.max_position_size = 10.0; // 10 contracts
        
        auto strategy = std::make_unique<OptionsStrategy>(strategy_config);
        
        // Run backtest
        BacktestResults results = engine_.runBacktest(config, std::move(strategy));
        
        // Generate report
        engine_.generatePerformanceReport(results, "options_strategy_backtest.txt");
        
        // Publish results
        publishBacktestResults(results);
    }
    
    void runStrategyComparison() {
        std::cout << "\n--- Strategy Comparison Analysis ---" << std::endl;
        
        std::vector<BacktestResults> all_results;
        
        // Run multiple strategies with same parameters for comparison
        std::vector<StrategyType> strategy_types = {
            StrategyType::MOMENTUM,
            StrategyType::OPTIONS_STRADDLE,
            StrategyType::IRON_CONDOR
        };
        
        for (auto type : strategy_types) {
            BacktestConfig config;
            config.strategy_name = getStrategyName(type) + "_Comparison";
            config.start_date = std::chrono::system_clock::now() - std::chrono::hours(24 * 252); // 1 trading year
            config.end_date = std::chrono::system_clock::now();
            config.symbols = {"AAPL", "GOOGL", "TSLA"};
            config.starting_capital = 1000000.0;
            config.commission_per_trade = 5.0;
            config.slippage_bps = 5.0;
            
            StrategyConfig strategy_config;
            strategy_config.type = type;
            strategy_config.name = getStrategyName(type);
            strategy_config.symbols = config.symbols;
            strategy_config.enabled = true;
            strategy_config.max_position_size = type == StrategyType::MOMENTUM ? 100.0 : 10.0;
            
            std::unique_ptr<TradingStrategy> strategy;
            if (type == StrategyType::MOMENTUM) {
                strategy = std::make_unique<MomentumStrategy>(strategy_config);
            } else {
                strategy = std::make_unique<OptionsStrategy>(strategy_config);
            }
            
            BacktestResults results = engine_.runBacktest(config, std::move(strategy));
            all_results.push_back(results);
        }
        
        // Generate comparison report
        engine_.generateComparisonReport(all_results, "strategy_comparison_report.txt");
        
        // Publish comparison results
        publishComparisonResults(all_results);
    }
    
    void handleBacktestRequest(const Message& msg) {
        std::cout << "Received backtest request: " << msg.payload << std::endl;
        
        // Parse request (simplified)
        // In production, would parse JSON request with strategy parameters
        
        BacktestConfig config;
        config.strategy_name = "Custom_Backtest";
        config.start_date = std::chrono::system_clock::now() - std::chrono::hours(24 * 90);
        config.end_date = std::chrono::system_clock::now();
        config.symbols = {"AAPL"};
        config.starting_capital = 100000.0;
        config.commission_per_trade = 5.0;
        
        StrategyConfig strategy_config;
        strategy_config.type = StrategyType::MOMENTUM;
        strategy_config.name = "Custom_Momentum";
        strategy_config.symbols = config.symbols;
        strategy_config.enabled = true;
        strategy_config.max_position_size = 100.0;
        
        auto strategy = std::make_unique<MomentumStrategy>(strategy_config);
        BacktestResults results = engine_.runBacktest(config, std::move(strategy));
        
        // Send results back
        publishBacktestResults(results);
    }
    
    void handleOptimizationRequest(const Message& msg) {
        std::cout << "Received optimization request: " << msg.payload << std::endl;
        
        // Run parameter optimization
        BacktestConfig base_config;
        base_config.strategy_name = "Optimization_Test";
        base_config.start_date = std::chrono::system_clock::now() - std::chrono::hours(24 * 180);
        base_config.end_date = std::chrono::system_clock::now();
        base_config.symbols = {"AAPL"};
        base_config.starting_capital = 100000.0;
        base_config.commission_per_trade = 5.0;
        
        // Parameter ranges for optimization
        std::unordered_map<std::string, std::vector<double>> parameter_ranges;
        parameter_ranges["momentum_threshold"] = {0.01, 0.015, 0.02, 0.025, 0.03};
        parameter_ranges["volatility_threshold"] = {0.02, 0.025, 0.03, 0.035, 0.04};
        
        StrategyConfig strategy_config;
        strategy_config.type = StrategyType::MOMENTUM;
        strategy_config.name = "Optimization_Strategy";
        strategy_config.symbols = base_config.symbols;
        strategy_config.enabled = true;
        strategy_config.max_position_size = 100.0;
        
        auto strategy = std::make_unique<MomentumStrategy>(strategy_config);
        BacktestResults best_results = engine_.optimizeStrategy(base_config, std::move(strategy), parameter_ranges);
        
        publishBacktestResults(best_results);
    }
    
    void publishBacktestResults(const BacktestResults& results) {
        std::ostringstream msg;
        msg << "BACKTEST_RESULTS," << results.strategy_name << ","
            << results.total_return << "," << results.sharpe_ratio << ","
            << results.max_drawdown << "," << results.win_rate << ","
            << results.total_trades << "," << results.profit_factor;
        
        mq_.publish("backtest.results", msg.str());
        
        std::cout << "Published backtest results for: " << results.strategy_name << std::endl;
    }
    
    void publishComparisonResults(const std::vector<BacktestResults>& results) {
        std::ostringstream msg;
        msg << "STRATEGY_COMPARISON,";
        
        for (size_t i = 0; i < results.size(); ++i) {
            if (i > 0) msg << ";";
            msg << results[i].strategy_name << ":" << results[i].total_return 
                << ":" << results[i].sharpe_ratio << ":" << results[i].max_drawdown;
        }
        
        mq_.publish("backtest.comparison", msg.str());
        
        std::cout << "Published strategy comparison results" << std::endl;
    }
    
    std::string getStrategyName(StrategyType type) {
        switch (type) {
            case StrategyType::MOMENTUM: return "Momentum";
            case StrategyType::OPTIONS_STRADDLE: return "Options_Straddle";
            case StrategyType::IRON_CONDOR: return "Iron_Condor";
            case StrategyType::COVERED_CALL: return "Covered_Call";
            default: return "Unknown";
        }
    }
};

int main() {
    std::cout << "Starting Backtesting Service..." << std::endl;
    
    BacktestingService service;
    
    if (!service.initialize()) {
        std::cerr << "Failed to initialize Backtesting Service" << std::endl;
        return 1;
    }
    
    service.run();
    return 0;
}