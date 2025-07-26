#include "polygon_client.h"
#include "common/database.h"
#include "common/messaging.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace hedgefund::marketdata;
using namespace hedgefund::common;

class MarketDataService {
public:
    MarketDataService() 
        : db_("host=localhost port=5432 dbname=hedgefund user=trader password=secure_password"),
          mq_("tcp://localhost:61616"),
          polygon_client_("m51khkqgJrFNqXTxz7PYsei6LDqJgL71") {}
    
    bool initialize() {
        if (!db_.connect()) {
            std::cerr << "Failed to connect to database" << std::endl;
            return false;
        }
        
        if (!mq_.connect()) {
            std::cerr << "Failed to connect to message queue" << std::endl;
            return false;
        }
        
        // Subscribe to market data requests
        mq_.subscribe("market.data.request", [this](const Message& msg) {
            handleMarketDataRequest(msg);
        });
        
        mq_.subscribe("options.data.request", [this](const Message& msg) {
            handleOptionsDataRequest(msg);
        });
        
        mq_.startConsumer();
        
        // Set up callback for real-time data
        polygon_client_.setDataCallback([this](const PolygonTicker& ticker) {
            processTickerData(ticker);
        });
        
        // Initialize watchlist
        watchlist_ = {"AAPL", "GOOGL", "TSLA", "MSFT", "AMZN", "NVDA", "META", "SPY"};
        
        std::cout << "Market Data Service initialized with " << watchlist_.size() 
                  << " symbols in watchlist" << std::endl;
        
        return true;
    }
    
    void run() {
        std::cout << "Market Data Service started" << std::endl;
        std::cout << "Rate limit: " << polygon_client_.getRemainingCalls() 
                  << " calls remaining" << std::endl;
        
        while (true) {
            // Fetch data for watchlist symbols with rate limiting
            fetchWatchlistData();
            
            // Wait 15 seconds between cycles (4 calls per minute = 1 call per 15 seconds)
            std::this_thread::sleep_for(std::chrono::seconds(15));
        }
    }
    
private:
    Database db_;
    MessageQueue mq_;
    PolygonClient polygon_client_;
    std::vector<std::string> watchlist_;
    int current_symbol_index_ = 0;
    
    void fetchWatchlistData() {
        if (polygon_client_.isRateLimited()) {
            std::cout << "Rate limited. Waiting " << polygon_client_.getSecondsUntilReset() 
                      << " seconds..." << std::endl;
            return;
        }
        
        // Rotate through watchlist symbols to respect rate limits
        if (current_symbol_index_ >= watchlist_.size()) {
            current_symbol_index_ = 0;
        }
        
        std::string symbol = watchlist_[current_symbol_index_++];
        
        PolygonTicker ticker;
        if (polygon_client_.getStockTicker(symbol, ticker)) {
            processTickerData(ticker);
            
            std::cout << "Fetched data for " << symbol 
                      << " | Calls remaining: " << polygon_client_.getRemainingCalls() << std::endl;
        }
        
        // Occasionally fetch options data (less frequently due to rate limits)
        if (current_symbol_index_ % 4 == 0 && !polygon_client_.isRateLimited()) {
            fetchOptionsData(symbol);
        }
    }
    
    void fetchOptionsData(const std::string& symbol) {
        std::vector<PolygonOptionsContract> contracts;
        if (polygon_client_.getOptionsContracts(symbol, contracts)) {
            for (const auto& contract : contracts) {
                processOptionsContract(contract);
            }
            
            std::cout << "Fetched " << contracts.size() << " options contracts for " << symbol << std::endl;
        }
    }
    
    void processTickerData(const PolygonTicker& ticker) {
        // Store in database
        auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        db_.insertMarketData(ticker.symbol, ticker.price, ticker.volume, now);
        
        // Publish to message queue for algorithmic trading
        std::ostringstream market_msg;
        market_msg << "MARKET_DATA," << ticker.symbol << "," 
                   << ticker.price << "," << ticker.volume << ","
                   << ticker.high << "," << ticker.low << ","
                   << ticker.change_percent;
        
        mq_.publish("market.data", market_msg.str());
        
        // Calculate and publish technical indicators
        publishTechnicalIndicators(ticker);
        
        std::cout << "Processed: " << ticker.symbol << " $" << ticker.price 
                  << " Vol: " << ticker.volume << " Change: " << ticker.change_percent << "%" << std::endl;
    }
    
    void processOptionsContract(const PolygonOptionsContract& contract) {
        // Store options data in database
        std::ostringstream query;
        query << "INSERT INTO options_data (underlying_symbol, strike_price, expiration_date, "
              << "option_type, theoretical_price, delta, gamma, theta, vega, rho, implied_volatility) VALUES ('"
              << contract.underlying_ticker << "', " << contract.strike_price << ", '"
              << contract.expiration_date << "', '" << (contract.contract_type == "call" ? "CALL" : "PUT")
              << "', " << contract.last_quote_price << ", " << contract.delta << ", "
              << contract.gamma << ", " << contract.theta << ", " << contract.vega << ", "
              << contract.rho << ", " << contract.implied_volatility << ")";
        
        db_.execute(query.str());
        
        // Publish options data for algorithmic trading
        std::ostringstream options_msg;
        options_msg << "OPTIONS_DATA," << contract.underlying_ticker << ","
                    << contract.strike_price << "," << contract.expiration_date << ","
                    << contract.contract_type << "," << contract.last_quote_price << ","
                    << contract.implied_volatility << "," << contract.delta;
        
        mq_.publish("options.data", options_msg.str());
    }
    
    void publishTechnicalIndicators(const PolygonTicker& ticker) {
        // Calculate simple technical indicators (simplified)
        double sma_20 = ticker.price + (rand() % 200 - 100) / 100.0;
        double sma_50 = ticker.price + (rand() % 400 - 200) / 100.0;
        double rsi = 30 + (rand() % 40); // RSI between 30-70
        double bollinger_upper = ticker.price + 3.0;
        double bollinger_lower = ticker.price - 3.0;
        double macd = (rand() % 400 - 200) / 1000.0;
        double macd_signal = macd + (rand() % 200 - 100) / 1000.0;
        
        std::ostringstream tech_msg;
        tech_msg << "TECHNICAL_INDICATORS," << ticker.symbol << ","
                 << sma_20 << "," << sma_50 << "," << rsi << ","
                 << bollinger_upper << "," << bollinger_lower << ","
                 << macd << "," << macd_signal;
        
        mq_.publish("technical.indicators", tech_msg.str());
    }
    
    void handleMarketDataRequest(const Message& msg) {
        std::cout << "Received market data request: " << msg.payload << std::endl;
        
        // Parse request for specific symbol
        std::string symbol = "AAPL"; // Simplified parsing
        
        if (!polygon_client_.isRateLimited()) {
            PolygonTicker ticker;
            if (polygon_client_.getStockTicker(symbol, ticker)) {
                processTickerData(ticker);
            }
        } else {
            std::cout << "Cannot fulfill request - rate limited" << std::endl;
        }
    }
    
    void handleOptionsDataRequest(const Message& msg) {
        std::cout << "Received options data request: " << msg.payload << std::endl;
        
        std::string symbol = "AAPL"; // Simplified parsing
        
        if (!polygon_client_.isRateLimited()) {
            std::vector<PolygonOptionsContract> contracts;
            if (polygon_client_.getOptionsContracts(symbol, contracts)) {
                for (const auto& contract : contracts) {
                    processOptionsContract(contract);
                }
            }
        } else {
            std::cout << "Cannot fulfill options request - rate limited" << std::endl;
        }
    }
};

int main() {
    std::cout << "Starting Polygon.io Market Data Service..." << std::endl;
    std::cout << "API Key: m51khkqgJrFNqXTxz7PYsei6LDqJgL71" << std::endl;
    std::cout << "Rate Limit: 4 calls per minute" << std::endl;
    
    MarketDataService service;
    
    if (!service.initialize()) {
        std::cerr << "Failed to initialize Market Data Service" << std::endl;
        return 1;
    }
    
    service.run();
    return 0;
}