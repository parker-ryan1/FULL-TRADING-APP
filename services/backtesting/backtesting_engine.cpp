#include "backtesting_engine.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <random>
#include <iomanip>

namespace hedgefund {
namespace backtesting {

// Technical Indicators Implementation
std::vector<double> TechnicalIndicators::calculateSMA(const std::vector<double>& prices, int period) {
    std::vector<double> sma;
    if (prices.size() < static_cast<size_t>(period)) return sma;
    
    for (size_t i = period - 1; i < prices.size(); ++i) {
        double sum = 0.0;
        for (int j = 0; j < period; ++j) {
            sum += prices[i - j];
        }
        sma.push_back(sum / period);
    }
    return sma;
}

std::vector<double> TechnicalIndicators::calculateEMA(const std::vector<double>& prices, int period) {
    std::vector<double> ema;
    if (prices.empty()) return ema;
    
    double multiplier = 2.0 / (period + 1);
    ema.push_back(prices[0]);
    
    for (size_t i = 1; i < prices.size(); ++i) {
        double value = (prices[i] * multiplier) + (ema.back() * (1 - multiplier));
        ema.push_back(value);
    }
    return ema;
}

std::vector<double> TechnicalIndicators::calculateRSI(const std::vector<double>& prices, int period) {
    std::vector<double> rsi;
    if (prices.size() < static_cast<size_t>(period + 1)) return rsi;
    
    std::vector<double> gains, losses;
    for (size_t i = 1; i < prices.size(); ++i) {
        double change = prices[i] - prices[i-1];
        gains.push_back(change > 0 ? change : 0);
        losses.push_back(change < 0 ? -change : 0);
    }
    
    // Calculate initial averages
    double avg_gain = std::accumulate(gains.begin(), gains.begin() + period, 0.0) / period;
    double avg_loss = std::accumulate(losses.begin(), losses.begin() + period, 0.0) / period;
    
    for (size_t i = period; i < gains.size(); ++i) {
        avg_gain = (avg_gain * (period - 1) + gains[i]) / period;
        avg_loss = (avg_loss * (period - 1) + losses[i]) / period;
        
        double rs = avg_loss == 0 ? 100 : avg_gain / avg_loss;
        rsi.push_back(100 - (100 / (1 + rs)));
    }
    return rsi;
}

double TechnicalIndicators::calculateBeta(const std::vector<double>& returns, const std::vector<double>& benchmark_returns) {
    if (returns.size() != benchmark_returns.size() || returns.size() < 2) return 1.0;
    
    double covariance = 0.0;
    double variance = 0.0;
    double mean_return = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double mean_benchmark = std::accumulate(benchmark_returns.begin(), benchmark_returns.end(), 0.0) / benchmark_returns.size();
    
    for (size_t i = 0; i < returns.size(); ++i) {
        covariance += (returns[i] - mean_return) * (benchmark_returns[i] - mean_benchmark);
        variance += (benchmark_returns[i] - mean_benchmark) * (benchmark_returns[i] - mean_benchmark);
    }
    
    return variance == 0 ? 1.0 : covariance / variance;
}

// Backtesting Engine Implementation
BacktestingEngine::BacktestingEngine() {
    std::cout << "Backtesting Engine initialized" << std::endl;
}

BacktestingEngine::~BacktestingEngine() = default;

bool BacktestingEngine::loadHistoricalDataFromCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    std::getline(file, line); // Skip header
    
    std::vector<HistoricalData> data;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;
        
        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }
        
        if (tokens.size() >= 7) {
            HistoricalData point;
            point.symbol = tokens[0];
            // Parse date (simplified)
            point.timestamp = std::chrono::system_clock::now();
            point.open = std::stod(tokens[2]);
            point.high = std::stod(tokens[3]);
            point.low = std::stod(tokens[4]);
            point.close = std::stod(tokens[5]);
            point.volume = std::stod(tokens[6]);
            point.adjusted_close = tokens.size() > 7 ? std::stod(tokens[7]) : point.close;
            
            data.push_back(point);
        }
    }
    
    if (!data.empty()) {
        std::string symbol = data[0].symbol;
        calculateTechnicalIndicators(data);
        historical_data_[symbol] = data;
        std::cout << "Loaded " << data.size() << " data points for " << symbol << std::endl;
        return true;
    }
    
    return false;
}

BacktestResults BacktestingEngine::runBacktest(const BacktestConfig& config, 
                                              std::unique_ptr<hedgefund::algo::TradingStrategy> strategy) {
    BacktestResults results;
    results.strategy_name = config.strategy_name;
    results.start_date = config.start_date;
    results.end_date = config.end_date;
    results.starting_capital = config.starting_capital;
    
    std::cout << "Running backtest for strategy: " << config.strategy_name << std::endl;
    
    // Simulate trading over historical data
    double current_capital = config.starting_capital;
    std::vector<BacktestTrade> trades;
    
    // Generate sample trades for demonstration
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> price_dist(100.0, 200.0);
    std::uniform_real_distribution<> return_dist(-0.05, 0.08);
    
    for (int i = 0; i < 100; ++i) {
        BacktestTrade trade;
        trade.strategy_id = config.strategy_name;
        trade.symbol = config.symbols.empty() ? "AAPL" : config.symbols[0];
        trade.side = (i % 2 == 0) ? "BUY" : "SELL";
        trade.entry_price = price_dist(gen);
        trade.quantity = 100;
        trade.commission = config.commission_per_trade;
        trade.entry_time = std::chrono::system_clock::now() - std::chrono::hours(24 * (100 - i));
        trade.exit_time = trade.entry_time + std::chrono::hours(24);
        
        double return_pct = return_dist(gen);
        trade.exit_price = trade.entry_price * (1 + return_pct);
        trade.return_pct = return_pct;
        trade.pnl = (trade.exit_price - trade.entry_price) * trade.quantity - trade.commission;
        
        trades.push_back(trade);
        current_capital += trade.pnl;
    }
    
    results.trades = trades;
    results.ending_capital = current_capital;
    
    // Calculate performance metrics
    std::vector<double> daily_returns = calculateDailyReturns(trades, config);
    results.daily_returns = daily_returns;
    results.equity_curve = calculateEquityCurve(trades, config.starting_capital);
    results.drawdown_curve = calculateDrawdownCurve(results.equity_curve);
    
    // Performance calculations
    results.total_return = (results.ending_capital - results.starting_capital) / results.starting_capital;
    results.annualized_return = std::pow(1 + results.total_return, 365.0 / 252.0) - 1; // Assuming 252 trading days
    results.volatility = calculateVolatility(daily_returns);
    results.sharpe_ratio = calculateSharpeRatio(daily_returns);
    results.sortino_ratio = calculateSortinoRatio(daily_returns);
    results.max_drawdown = calculateMaxDrawdown(results.equity_curve);
    results.var_95 = calculateVaR(daily_returns);
    results.cvar_95 = calculateCVaR(daily_returns);
    
    // Trading statistics
    results.total_trades = trades.size();
    results.winning_trades = std::count_if(trades.begin(), trades.end(), 
        [](const BacktestTrade& t) { return t.pnl > 0; });
    results.losing_trades = results.total_trades - results.winning_trades;
    results.win_rate = static_cast<double>(results.winning_trades) / results.total_trades;
    
    // Calculate average win/loss
    double total_wins = 0, total_losses = 0;
    for (const auto& trade : trades) {
        if (trade.pnl > 0) total_wins += trade.pnl;
        else total_losses += std::abs(trade.pnl);
    }
    results.avg_win = results.winning_trades > 0 ? total_wins / results.winning_trades : 0;
    results.avg_loss = results.losing_trades > 0 ? total_losses / results.losing_trades : 0;
    results.profit_factor = total_losses > 0 ? total_wins / total_losses : 0;
    
    std::cout << "Backtest completed. Total return: " << (results.total_return * 100) << "%" << std::endl;
    std::cout << "Sharpe ratio: " << results.sharpe_ratio << std::endl;
    std::cout << "Max drawdown: " << (results.max_drawdown * 100) << "%" << std::endl;
    
    return results;
}

void BacktestingEngine::generatePerformanceReport(const BacktestResults& results, const std::string& output_file) {
    std::ofstream file(output_file);
    if (!file.is_open()) {
        std::cerr << "Failed to create report file: " << output_file << std::endl;
        return;
    }
    
    file << "=== BACKTESTING PERFORMANCE REPORT ===" << std::endl;
    file << "Strategy: " << results.strategy_name << std::endl;
    auto start_time = std::chrono::system_clock::to_time_t(results.start_date);
    auto end_time = std::chrono::system_clock::to_time_t(results.end_date);
    file << "Period: " << std::put_time(std::localtime(&start_time), "%Y-%m-%d");
    file << " to " << std::put_time(std::localtime(&end_time), "%Y-%m-%d") << std::endl;
    file << std::endl;
    
    file << "=== PERFORMANCE METRICS ===" << std::endl;
    file << "Starting Capital: $" << std::fixed << std::setprecision(2) << results.starting_capital << std::endl;
    file << "Ending Capital: $" << results.ending_capital << std::endl;
    file << "Total Return: " << std::setprecision(2) << (results.total_return * 100) << "%" << std::endl;
    file << "Annualized Return: " << (results.annualized_return * 100) << "%" << std::endl;
    file << "Volatility: " << (results.volatility * 100) << "%" << std::endl;
    file << "Sharpe Ratio: " << std::setprecision(3) << results.sharpe_ratio << std::endl;
    file << "Sortino Ratio: " << results.sortino_ratio << std::endl;
    file << "Max Drawdown: " << std::setprecision(2) << (results.max_drawdown * 100) << "%" << std::endl;
    file << "VaR (95%): " << (results.var_95 * 100) << "%" << std::endl;
    file << "CVaR (95%): " << (results.cvar_95 * 100) << "%" << std::endl;
    file << std::endl;
    
    file << "=== TRADING STATISTICS ===" << std::endl;
    file << "Total Trades: " << results.total_trades << std::endl;
    file << "Winning Trades: " << results.winning_trades << std::endl;
    file << "Losing Trades: " << results.losing_trades << std::endl;
    file << "Win Rate: " << std::setprecision(1) << (results.win_rate * 100) << "%" << std::endl;
    file << "Average Win: $" << std::setprecision(2) << results.avg_win << std::endl;
    file << "Average Loss: $" << results.avg_loss << std::endl;
    file << "Profit Factor: " << std::setprecision(3) << results.profit_factor << std::endl;
    
    file.close();
    std::cout << "Performance report generated: " << output_file << std::endl;
}

// Helper methods implementation
void BacktestingEngine::calculateTechnicalIndicators(std::vector<HistoricalData>& data) {
    if (data.empty()) return;
    
    std::vector<double> closes;
    for (const auto& point : data) {
        closes.push_back(point.close);
    }
    
    auto sma_20 = TechnicalIndicators::calculateSMA(closes, 20);
    auto sma_50 = TechnicalIndicators::calculateSMA(closes, 50);
    auto rsi = TechnicalIndicators::calculateRSI(closes, 14);
    
    // Apply indicators to data (simplified)
    for (size_t i = 0; i < data.size(); ++i) {
        data[i].sma_20 = i >= 19 && i - 19 < sma_20.size() ? sma_20[i - 19] : data[i].close;
        data[i].sma_50 = i >= 49 && i - 49 < sma_50.size() ? sma_50[i - 49] : data[i].close;
        data[i].rsi = i >= 14 && i - 14 < rsi.size() ? rsi[i - 14] : 50.0;
        data[i].bollinger_upper = data[i].close + (data[i].close * 0.02);
        data[i].bollinger_lower = data[i].close - (data[i].close * 0.02);
        data[i].macd = 0.0;
        data[i].macd_signal = 0.0;
        data[i].atr = data[i].high - data[i].low;
        data[i].vwap = data[i].close;
    }
}

std::vector<double> BacktestingEngine::calculateDailyReturns(const std::vector<BacktestTrade>& trades, const BacktestConfig& config) {
    std::vector<double> returns;
    double current_value = config.starting_capital;
    
    for (const auto& trade : trades) {
        double return_pct = trade.pnl / current_value;
        returns.push_back(return_pct);
        current_value += trade.pnl;
    }
    
    return returns;
}

std::vector<double> BacktestingEngine::calculateEquityCurve(const std::vector<BacktestTrade>& trades, double starting_capital) {
    std::vector<double> equity;
    double current_equity = starting_capital;
    equity.push_back(current_equity);
    
    for (const auto& trade : trades) {
        current_equity += trade.pnl;
        equity.push_back(current_equity);
    }
    
    return equity;
}

double BacktestingEngine::calculateSharpeRatio(const std::vector<double>& returns, double risk_free_rate) {
    if (returns.empty()) return 0.0;
    
    double mean_return = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double excess_return = mean_return - (risk_free_rate / 252.0); // Daily risk-free rate
    
    double variance = 0.0;
    for (double ret : returns) {
        variance += (ret - mean_return) * (ret - mean_return);
    }
    variance /= returns.size();
    double volatility = std::sqrt(variance);
    
    return volatility == 0 ? 0.0 : excess_return / volatility;
}

double BacktestingEngine::calculateMaxDrawdown(const std::vector<double>& equity_curve) {
    if (equity_curve.empty()) return 0.0;
    
    double max_drawdown = 0.0;
    double peak = equity_curve[0];
    
    for (double value : equity_curve) {
        if (value > peak) peak = value;
        double drawdown = (peak - value) / peak;
        max_drawdown = std::max(max_drawdown, drawdown);
    }
    
    return max_drawdown;
}

double BacktestingEngine::calculateVaR(const std::vector<double>& returns, double confidence) {
    if (returns.empty()) return 0.0;
    
    std::vector<double> sorted_returns = returns;
    std::sort(sorted_returns.begin(), sorted_returns.end());
    
    int index = static_cast<int>((1.0 - confidence) * sorted_returns.size());
    index = std::max(0, std::min(index, static_cast<int>(sorted_returns.size()) - 1));
    
    return -sorted_returns[index];
}

double BacktestingEngine::calculateVolatility(const std::vector<double>& returns) {
    if (returns.empty()) return 0.0;
    
    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double variance = 0.0;
    
    for (double ret : returns) {
        variance += (ret - mean) * (ret - mean);
    }
    variance /= returns.size();
    
    return std::sqrt(variance * 252); // Annualized volatility
}

double BacktestingEngine::calculateSortinoRatio(const std::vector<double>& returns, double risk_free_rate) {
    if (returns.empty()) return 0.0;
    
    double mean_return = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double excess_return = mean_return - (risk_free_rate / 252.0);
    
    double downside_variance = 0.0;
    int downside_count = 0;
    
    for (double ret : returns) {
        if (ret < 0) {
            downside_variance += ret * ret;
            downside_count++;
        }
    }
    
    if (downside_count == 0) return 0.0;
    
    double downside_deviation = std::sqrt(downside_variance / downside_count);
    return downside_deviation == 0 ? 0.0 : excess_return / downside_deviation;
}

double BacktestingEngine::calculateCVaR(const std::vector<double>& returns, double confidence) {
    if (returns.empty()) return 0.0;
    
    std::vector<double> sorted_returns = returns;
    std::sort(sorted_returns.begin(), sorted_returns.end());
    
    int var_index = static_cast<int>((1.0 - confidence) * sorted_returns.size());
    var_index = std::max(0, std::min(var_index, static_cast<int>(sorted_returns.size()) - 1));
    
    double sum = 0.0;
    for (int i = 0; i <= var_index; ++i) {
        sum += sorted_returns[i];
    }
    
    return -(sum / (var_index + 1));
}

std::vector<double> BacktestingEngine::calculateDrawdownCurve(const std::vector<double>& equity_curve) {
    std::vector<double> drawdown_curve;
    if (equity_curve.empty()) return drawdown_curve;
    
    double peak = equity_curve[0];
    for (double value : equity_curve) {
        if (value > peak) peak = value;
        double drawdown = (peak - value) / peak;
        drawdown_curve.push_back(drawdown);
    }
    
    return drawdown_curve;
}

BacktestResults BacktestingEngine::optimizeStrategy(const BacktestConfig& base_config,
                                                   std::unique_ptr<hedgefund::algo::TradingStrategy> strategy,
                                                   const std::unordered_map<std::string, std::vector<double>>& parameter_ranges) {
    BacktestResults best_results;
    best_results.sharpe_ratio = -999.0; // Initialize with very low value
    
    std::cout << "Running strategy optimization..." << std::endl;
    
    // Simple grid search optimization (simplified)
    for (const auto& [param_name, values] : parameter_ranges) {
        for (double value : values) {
            BacktestConfig test_config = base_config;
            test_config.parameters[param_name] = value;
            test_config.strategy_name = base_config.strategy_name + "_" + param_name + "_" + std::to_string(value);
            
            // Create new strategy instance with updated parameters
            // Note: In a real implementation, we'd need to properly clone the strategy
            BacktestResults results = runBacktest(test_config, std::move(strategy));
            
            if (results.sharpe_ratio > best_results.sharpe_ratio) {
                best_results = results;
                std::cout << "New best parameters: " << param_name << "=" << value 
                          << ", Sharpe: " << results.sharpe_ratio << std::endl;
            }
            
            // Break after first iteration for demo (would need proper strategy cloning)
            break;
        }
        break; // Break after first parameter for demo
    }
    
    return best_results;
}

void BacktestingEngine::generateComparisonReport(const std::vector<BacktestResults>& results, const std::string& output_file) {
    std::ofstream file(output_file);
    if (!file.is_open()) {
        std::cerr << "Failed to create comparison report: " << output_file << std::endl;
        return;
    }
    
    file << "=== STRATEGY COMPARISON REPORT ===" << std::endl;
    file << std::endl;
    
    file << std::left << std::setw(20) << "Strategy" 
         << std::setw(15) << "Total Return" 
         << std::setw(15) << "Sharpe Ratio" 
         << std::setw(15) << "Max Drawdown" 
         << std::setw(15) << "Win Rate" 
         << std::setw(15) << "Total Trades" << std::endl;
    
    file << std::string(95, '-') << std::endl;
    
    for (const auto& result : results) {
        file << std::left << std::setw(20) << result.strategy_name
             << std::setw(15) << std::fixed << std::setprecision(2) << (result.total_return * 100) << "%"
             << std::setw(15) << std::setprecision(3) << result.sharpe_ratio
             << std::setw(15) << std::setprecision(2) << (result.max_drawdown * 100) << "%"
             << std::setw(15) << std::setprecision(1) << (result.win_rate * 100) << "%"
             << std::setw(15) << result.total_trades << std::endl;
    }
    
    file.close();
    std::cout << "Strategy comparison report generated: " << output_file << std::endl;
}

} // namespace backtesting
} // namespace hedgefund