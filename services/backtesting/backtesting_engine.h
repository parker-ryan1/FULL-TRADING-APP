#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <unordered_map>
#include "../algo-trading/algo_engine.h"

namespace hedgefund {
namespace backtesting {

struct HistoricalData {
    std::string symbol;
    double open;
    double high;
    double low;
    double close;
    double volume;
    double adjusted_close;
    std::chrono::system_clock::time_point timestamp;
    
    // Technical indicators
    double sma_20;
    double sma_50;
    double sma_200;
    double rsi;
    double bollinger_upper;
    double bollinger_lower;
    double macd;
    double macd_signal;
    double atr; // Average True Range
    double vwap; // Volume Weighted Average Price
};

struct BacktestTrade {
    std::string strategy_id;
    std::string symbol;
    std::string side; // "BUY" or "SELL"
    double entry_price;
    double exit_price;
    double quantity;
    double commission;
    std::chrono::system_clock::time_point entry_time;
    std::chrono::system_clock::time_point exit_time;
    double pnl;
    double return_pct;
    
    // Options specific
    bool is_option = false;
    double strike_price = 0.0;
    std::string expiration_date;
    bool is_call = true;
};

struct BacktestResults {
    std::string strategy_name;
    std::chrono::system_clock::time_point start_date;
    std::chrono::system_clock::time_point end_date;
    
    // Performance metrics
    double total_return;
    double annualized_return;
    double volatility;
    double sharpe_ratio;
    double sortino_ratio;
    double max_drawdown;
    double max_drawdown_duration; // days
    double calmar_ratio;
    double var_95; // Value at Risk 95%
    double cvar_95; // Conditional VaR 95%
    
    // Trading statistics
    int total_trades;
    int winning_trades;
    int losing_trades;
    double win_rate;
    double avg_win;
    double avg_loss;
    double profit_factor;
    double largest_win;
    double largest_loss;
    
    // Portfolio metrics
    double starting_capital;
    double ending_capital;
    double peak_capital;
    double total_commission;
    double total_slippage;
    
    // Risk metrics
    double beta;
    double alpha;
    double information_ratio;
    double tracking_error;
    
    std::vector<BacktestTrade> trades;
    std::vector<double> daily_returns;
    std::vector<double> equity_curve;
    std::vector<double> drawdown_curve;
};

struct BacktestConfig {
    std::string strategy_name;
    std::chrono::system_clock::time_point start_date;
    std::chrono::system_clock::time_point end_date;
    std::vector<std::string> symbols;
    double starting_capital;
    double commission_per_trade;
    double slippage_bps; // basis points
    bool allow_short_selling;
    double max_position_size; // as fraction of capital
    std::string benchmark_symbol; // for beta calculation
    
    // Strategy parameters
    std::unordered_map<std::string, double> parameters;
};

class TechnicalIndicators {
public:
    static std::vector<double> calculateSMA(const std::vector<double>& prices, int period);
    static std::vector<double> calculateEMA(const std::vector<double>& prices, int period);
    static std::vector<double> calculateRSI(const std::vector<double>& prices, int period = 14);
    static std::vector<double> calculateMACD(const std::vector<double>& prices, int fast = 12, int slow = 26, int signal = 9);
    static std::vector<double> calculateBollingerBands(const std::vector<double>& prices, int period = 20, double std_dev = 2.0);
    static std::vector<double> calculateATR(const std::vector<HistoricalData>& data, int period = 14);
    static std::vector<double> calculateVWAP(const std::vector<HistoricalData>& data);
    static double calculateBeta(const std::vector<double>& returns, const std::vector<double>& benchmark_returns);
    static double calculateCorrelation(const std::vector<double>& x, const std::vector<double>& y);
};

class BacktestingEngine {
public:
    BacktestingEngine();
    ~BacktestingEngine();
    
    // Data management
    bool loadHistoricalData(const std::string& symbol, const std::string& start_date, const std::string& end_date);
    bool loadHistoricalDataFromCSV(const std::string& filename);
    bool loadHistoricalDataFromDatabase(const std::vector<std::string>& symbols, 
                                       const std::string& start_date, const std::string& end_date);
    
    // Strategy testing
    BacktestResults runBacktest(const BacktestConfig& config, 
                               std::unique_ptr<hedgefund::algo::TradingStrategy> strategy);
    
    // Multi-strategy testing
    std::vector<BacktestResults> runMultiStrategyBacktest(const std::vector<BacktestConfig>& configs,
                                                         std::vector<std::unique_ptr<hedgefund::algo::TradingStrategy>> strategies);
    
    // Walk-forward analysis
    std::vector<BacktestResults> runWalkForwardAnalysis(const BacktestConfig& config,
                                                       std::unique_ptr<hedgefund::algo::TradingStrategy> strategy,
                                                       int training_days, int testing_days);
    
    // Monte Carlo simulation
    std::vector<BacktestResults> runMonteCarloSimulation(const BacktestConfig& config,
                                                        std::unique_ptr<hedgefund::algo::TradingStrategy> strategy,
                                                        int num_simulations);
    
    // Performance analysis
    void generatePerformanceReport(const BacktestResults& results, const std::string& output_file);
    void generateComparisonReport(const std::vector<BacktestResults>& results, const std::string& output_file);
    
    // Optimization
    BacktestResults optimizeStrategy(const BacktestConfig& base_config,
                                   std::unique_ptr<hedgefund::algo::TradingStrategy> strategy,
                                   const std::unordered_map<std::string, std::vector<double>>& parameter_ranges);
    
private:
    std::unordered_map<std::string, std::vector<HistoricalData>> historical_data_;
    
    // Helper methods
    void calculateTechnicalIndicators(std::vector<HistoricalData>& data);
    BacktestResults calculatePerformanceMetrics(const std::vector<BacktestTrade>& trades,
                                               const std::vector<double>& daily_returns,
                                               const BacktestConfig& config);
    std::vector<double> calculateDailyReturns(const std::vector<BacktestTrade>& trades,
                                             const BacktestConfig& config);
    std::vector<double> calculateEquityCurve(const std::vector<BacktestTrade>& trades,
                                            double starting_capital);
    std::vector<double> calculateDrawdownCurve(const std::vector<double>& equity_curve);
    
    double calculateSharpeRatio(const std::vector<double>& returns, double risk_free_rate = 0.02);
    double calculateSortinoRatio(const std::vector<double>& returns, double risk_free_rate = 0.02);
    double calculateMaxDrawdown(const std::vector<double>& equity_curve);
    double calculateVaR(const std::vector<double>& returns, double confidence = 0.95);
    double calculateCVaR(const std::vector<double>& returns, double confidence = 0.95);
    double calculateVolatility(const std::vector<double>& returns);
};

} // namespace backtesting
} // namespace hedgefund