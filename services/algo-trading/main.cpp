#include "algo_engine.h"
#include "momentum_strategy.h"
#include "options_strategy.h"
#include "common/database.h"
#include "common/messaging.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <sstream>

using namespace hedgefund::algo;
using namespace hedgefund::common;

class AlgorithmicTradingService {
public:
    AlgorithmicTradingService() 
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
        
        if (!engine_.initialize()) {
            std::cerr << "Failed to initialize algorithmic engine" << std::endl;
            return false;
        }
        
        // Subscribe to market data from Polygon.io
        mq_.subscribe("market.data", [this](const Message& msg) {
            handleMarketData(msg);
        });
        
        // Subscribe to technical indicators
        mq_.subscribe("technical.indicators", [this](const Message& msg) {
            handleTechnicalIndicators(msg);
        });
        
        // Subscribe to options data
        mq_.subscribe("options.data", [this](const Message& msg) {
            handleOptionsData(msg);
        });
        
        // Subscribe to order execution confirmations
        mq_.subscribe("trades.executed", [this](const Message& msg) {
            handleTradeExecution(msg);
        });
        
        mq_.startConsumer();
        
        // Initialize trading strategies
        setupStrategies();
        
        return true;
    }
    
    void run() {
        std::cout << "Algorithmic Trading Service started" << std::endl;
        
        // Start the algorithmic engine in a separate thread
        std::thread engine_thread([this]() {
            engine_.run();
        });
        
        // Simulate market data for demonstration
        simulateMarketData();
        
        engine_thread.join();
    }
    
private:
    Database db_;
    MessageQueue mq_;
    AlgorithmicEngine engine_;
    std::mt19937 rng_{std::random_device{}()};
    
    void setupStrategies() {
        // Setup Momentum Strategy
        StrategyConfig momentum_config;
        momentum_config.type = StrategyType::MOMENTUM;
        momentum_config.name = "MomentumStrategy_1";
        momentum_config.parameters["momentum_threshold"] = 0.02;
        momentum_config.parameters["volatility_threshold"] = 0.03;
        momentum_config.symbols = {"AAPL", "GOOGL", "TSLA"};
        momentum_config.enabled = true;
        momentum_config.max_position_size = 100.0;
        momentum_config.stop_loss_pct = 0.05;
        momentum_config.take_profit_pct = 0.10;
        
        auto momentum_strategy = std::make_unique<MomentumStrategy>(momentum_config);
        engine_.addStrategy(std::move(momentum_strategy));
        
        // Setup Options Straddle Strategy
        StrategyConfig straddle_config;
        straddle_config.type = StrategyType::OPTIONS_STRADDLE;
        straddle_config.name = "OptionsStraddle_1";
        straddle_config.symbols = {"AAPL", "GOOGL"};
        straddle_config.enabled = true;
        straddle_config.max_position_size = 10.0; // 10 contracts
        straddle_config.stop_loss_pct = 0.50;
        straddle_config.take_profit_pct = 1.00;
        
        auto straddle_strategy = std::make_unique<OptionsStrategy>(straddle_config);
        engine_.addStrategy(std::move(straddle_strategy));
        
        // Setup Iron Condor Strategy
        StrategyConfig condor_config;
        condor_config.type = StrategyType::IRON_CONDOR;
        condor_config.name = "IronCondor_1";
        condor_config.symbols = {"AAPL", "TSLA"};
        condor_config.enabled = true;
        condor_config.max_position_size = 5.0; // 5 contracts
        condor_config.stop_loss_pct = 0.30;
        condor_config.take_profit_pct = 0.50;
        
        auto condor_strategy = std::make_unique<OptionsStrategy>(condor_config);
        engine_.addStrategy(std::move(condor_strategy));
        
        std::cout << "Initialized 3 trading strategies" << std::endl;
    }
    
    void handleMarketData(const Message& msg) {
        // Parse Polygon.io market data message
        // Format: "MARKET_DATA,SYMBOL,PRICE,VOLUME,HIGH,LOW,CHANGE_PERCENT"
        std::string payload = msg.payload;
        
        if (payload.find("MARKET_DATA") == 0) {
            std::istringstream ss(payload);
            std::string token;
            std::vector<std::string> tokens;
            
            while (std::getline(ss, token, ',')) {
                tokens.push_back(token);
            }
            
            if (tokens.size() >= 7) {
                MarketData data;
                data.symbol = tokens[1];
                data.price = std::stod(tokens[2]);
                data.volume = std::stod(tokens[3]);
                data.bid = data.price - 0.01; // Approximate bid
                data.ask = data.price + 0.01; // Approximate ask
                data.timestamp = std::chrono::system_clock::now();
                
                // Will be updated by technical indicators handler
                data.sma_20 = data.price;
                data.sma_50 = data.price;
                data.rsi = 50.0;
                data.bollinger_upper = data.price + 2.0;
                data.bollinger_lower = data.price - 2.0;
                data.macd = 0.0;
                data.macd_signal = 0.0;
                
                engine_.processMarketData(data);
                
                std::cout << "Processed Polygon.io data: " << data.symbol 
                          << " $" << data.price << " Vol: " << data.volume << std::endl;
            }
        }
    }
    
    void handleTechnicalIndicators(const Message& msg) {
        // Parse technical indicators from Polygon.io service
        // Format: "TECHNICAL_INDICATORS,SYMBOL,SMA20,SMA50,RSI,BB_UPPER,BB_LOWER,MACD,MACD_SIGNAL"
        std::string payload = msg.payload;
        
        if (payload.find("TECHNICAL_INDICATORS") == 0) {
            std::istringstream ss(payload);
            std::string token;
            std::vector<std::string> tokens;
            
            while (std::getline(ss, token, ',')) {
                tokens.push_back(token);
            }
            
            if (tokens.size() >= 9) {
                MarketData data;
                data.symbol = tokens[1];
                data.price = 0.0; // Will be updated from market data
                data.sma_20 = std::stod(tokens[2]);
                data.sma_50 = std::stod(tokens[3]);
                data.rsi = std::stod(tokens[4]);
                data.bollinger_upper = std::stod(tokens[5]);
                data.bollinger_lower = std::stod(tokens[6]);
                data.macd = std::stod(tokens[7]);
                data.macd_signal = std::stod(tokens[8]);
                data.timestamp = std::chrono::system_clock::now();
                
                // Update existing market data with technical indicators
                engine_.processMarketData(data);
                
                std::cout << "Updated technical indicators for " << data.symbol 
                          << " RSI: " << data.rsi << " MACD: " << data.macd << std::endl;
            }
        }
    }
    
    void handleOptionsData(const Message& msg) {
        // Parse options data from Polygon.io
        // Format: "OPTIONS_DATA,UNDERLYING,STRIKE,EXPIRATION,TYPE,PRICE,IV,DELTA"
        std::string payload = msg.payload;
        
        if (payload.find("OPTIONS_DATA") == 0) {
            std::istringstream ss(payload);
            std::string token;
            std::vector<std::string> tokens;
            
            while (std::getline(ss, token, ',')) {
                tokens.push_back(token);
            }
            
            if (tokens.size() >= 8) {
                std::string underlying = tokens[1];
                double strike = std::stod(tokens[2]);
                std::string expiration = tokens[3];
                std::string type = tokens[4];
                double price = std::stod(tokens[5]);
                double iv = std::stod(tokens[6]);
                double delta = std::stod(tokens[7]);
                
                std::cout << "Received options data: " << underlying 
                          << " " << strike << " " << type 
                          << " Price: $" << price << " IV: " << (iv * 100) << "%" << std::endl;
                
                // Store in database for options strategies
                std::ostringstream query;
                query << "INSERT INTO options_data (underlying_symbol, strike_price, expiration_date, "
                      << "option_type, theoretical_price, implied_volatility, delta) VALUES ('"
                      << underlying << "', " << strike << ", '" << expiration << "', '"
                      << type << "', " << price << ", " << iv << ", " << delta << ") "
                      << "ON CONFLICT (underlying_symbol, strike_price, expiration_date, option_type) "
                      << "DO UPDATE SET theoretical_price = " << price << ", implied_volatility = " << iv;
                
                db_.execute(query.str());
            }
        }
    }
    
    void handleTradeExecution(const Message& msg) {
        std::cout << "Trade executed: " << msg.payload << std::endl;
        
        // Update positions in database
        db_.execute("UPDATE positions SET quantity = quantity + 100 WHERE symbol = 'AAPL'");
    }
    
    void simulateMarketData() {
        std::vector<std::string> symbols = {"AAPL", "GOOGL", "TSLA"};
        
        while (true) {
            for (const auto& symbol : symbols) {
                MarketData data;
                data.symbol = symbol;
                data.price = 150.0 + (rng_() % 2000 - 1000) / 100.0; // Price between $140-$160
                data.volume = 1000 + (rng_() % 10000);
                data.bid = data.price - 0.05;
                data.ask = data.price + 0.05;
                data.timestamp = std::chrono::system_clock::now();
                
                // Simulate technical indicators
                data.sma_20 = data.price + (rng_() % 200 - 100) / 100.0;
                data.sma_50 = data.price + (rng_() % 400 - 200) / 100.0;
                data.rsi = 20 + (rng_() % 60); // RSI between 20-80
                data.bollinger_upper = data.price + 3.0 + (rng_() % 200) / 100.0;
                data.bollinger_lower = data.price - 3.0 - (rng_() % 200) / 100.0;
                data.macd = (rng_() % 400 - 200) / 1000.0;
                data.macd_signal = data.macd + (rng_() % 200 - 100) / 1000.0;
                
                engine_.processMarketData(data);
                
                // Publish market data to message queue
                std::ostringstream market_msg;
                market_msg << "MARKET_DATA," << symbol << "," << data.bid << "," 
                          << data.ask << "," << (data.ask - data.bid);
                mq_.publish("market.data", market_msg.str());
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
};

int main() {
    AlgorithmicTradingService service;
    
    if (!service.initialize()) {
        std::cerr << "Failed to initialize Algorithmic Trading Service" << std::endl;
        return 1;
    }
    
    service.run();
    return 0;
}