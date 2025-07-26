#include "black_scholes.h"
#include "brownian_motion.h"
#include "common/database.h"
#include "common/messaging.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>

using namespace hedgefund::options;
using namespace hedgefund::common;

class OptionsService {
public:
    OptionsService() 
        : db_("host=localhost port=5432 dbname=hedgefund user=trader password=secure_password"),
          mq_("tcp://localhost:61616"),
          brownian_motion_(std::random_device{}()) {}
    
    bool initialize() {
        if (!db_.connect()) {
            std::cerr << "Failed to connect to database" << std::endl;
            return false;
        }
        
        if (!mq_.connect()) {
            std::cerr << "Failed to connect to message queue" << std::endl;
            return false;
        }
        
        // Subscribe to pricing requests
        mq_.subscribe("options.price_request", [this](const Message& msg) {
            handlePriceRequest(msg);
        });
        
        mq_.subscribe("options.greeks_request", [this](const Message& msg) {
            handleGreeksRequest(msg);
        });
        
        mq_.subscribe("options.implied_vol_request", [this](const Message& msg) {
            handleImpliedVolRequest(msg);
        });
        
        mq_.startConsumer();
        return true;
    }
    
    void run() {
        std::cout << "Options Pricing Service started" << std::endl;
        
        // Demonstrate pricing capabilities
        demonstratePricing();
        
        while (true) {
            // Periodic tasks
            updateVolatilitySurface();
            std::this_thread::sleep_for(std::chrono::seconds(30));
        }
    }
    
private:
    Database db_;
    MessageQueue mq_;
    BrownianMotion brownian_motion_;
    
    void handlePriceRequest(const Message& msg) {
        // Parse pricing request (simplified JSON-like format)
        // Format: "SYMBOL,STRIKE,EXPIRY,IS_CALL,SPOT,VOL,RATE"
        std::cout << "Pricing request: " << msg.payload << std::endl;
        
        // Example parsing (in real implementation, use JSON)
        OptionParams params;
        params.spot_price = 150.0;
        params.strike_price = 155.0;
        params.time_to_expiry = 0.25; // 3 months
        params.risk_free_rate = 0.05;
        params.volatility = 0.20;
        params.is_call = true;
        
        // Calculate Black-Scholes price
        double bs_price = BlackScholes::calculatePrice(params);
        
        // Calculate Monte Carlo price for comparison
        MonteCarloParams mc_params;
        mc_params.spot_price = params.spot_price;
        mc_params.strike_price = params.strike_price;
        mc_params.time_to_expiry = params.time_to_expiry;
        mc_params.risk_free_rate = params.risk_free_rate;
        mc_params.volatility = params.volatility;
        mc_params.is_call = params.is_call;
        mc_params.num_simulations = 100000;
        mc_params.num_steps = 252; // Daily steps for one year
        
        SimulationResult mc_result = brownian_motion_.priceOption(mc_params);
        
        // Publish results
        std::ostringstream response;
        response << std::fixed << std::setprecision(4);
        response << "PRICE_RESPONSE," << bs_price << "," << mc_result.option_price 
                 << "," << mc_result.standard_error << "," << msg.correlation_id;
        
        mq_.publish("options.price_response", response.str());
        
        std::cout << "Black-Scholes Price: $" << bs_price << std::endl;
        std::cout << "Monte Carlo Price: $" << mc_result.option_price 
                  << " ± $" << mc_result.standard_error << std::endl;
    }
    
    void handleGreeksRequest(const Message& msg) {
        std::cout << "Greeks request: " << msg.payload << std::endl;
        
        OptionParams params;
        params.spot_price = 150.0;
        params.strike_price = 155.0;
        params.time_to_expiry = 0.25;
        params.risk_free_rate = 0.05;
        params.volatility = 0.20;
        params.is_call = true;
        
        Greeks greeks = BlackScholes::calculateGreeks(params);
        
        std::ostringstream response;
        response << std::fixed << std::setprecision(6);
        response << "GREEKS_RESPONSE," << greeks.delta << "," << greeks.gamma 
                 << "," << greeks.theta << "," << greeks.vega << "," << greeks.rho
                 << "," << msg.correlation_id;
        
        mq_.publish("options.greeks_response", response.str());
        
        std::cout << "Greeks - Delta: " << greeks.delta << ", Gamma: " << greeks.gamma
                  << ", Theta: " << greeks.theta << ", Vega: " << greeks.vega
                  << ", Rho: " << greeks.rho << std::endl;
    }
    
    void handleImpliedVolRequest(const Message& msg) {
        std::cout << "Implied volatility request: " << msg.payload << std::endl;
        
        OptionParams params;
        params.spot_price = 150.0;
        params.strike_price = 155.0;
        params.time_to_expiry = 0.25;
        params.risk_free_rate = 0.05;
        params.is_call = true;
        
        double market_price = 8.50; // Example market price
        double implied_vol = BlackScholes::impliedVolatility(market_price, params);
        
        std::ostringstream response;
        response << std::fixed << std::setprecision(4);
        response << "IMPLIED_VOL_RESPONSE," << implied_vol << "," << msg.correlation_id;
        
        mq_.publish("options.implied_vol_response", response.str());
        
        std::cout << "Implied Volatility: " << (implied_vol * 100) << "%" << std::endl;
    }
    
    void demonstratePricing() {
        std::cout << "\n=== Options Pricing Demonstration ===" << std::endl;
        
        OptionParams params;
        params.spot_price = 100.0;
        params.strike_price = 105.0;
        params.time_to_expiry = 0.25;
        params.risk_free_rate = 0.05;
        params.volatility = 0.20;
        params.is_call = true;
        
        // Black-Scholes pricing
        double call_price = BlackScholes::calculatePrice(params);
        params.is_call = false;
        double put_price = BlackScholes::calculatePrice(params);
        
        std::cout << "Call Option Price: $" << std::fixed << std::setprecision(2) << call_price << std::endl;
        std::cout << "Put Option Price: $" << put_price << std::endl;
        
        // Greeks calculation
        params.is_call = true;
        Greeks greeks = BlackScholes::calculateGreeks(params);
        std::cout << "Call Delta: " << std::setprecision(4) << greeks.delta << std::endl;
        std::cout << "Gamma: " << greeks.gamma << std::endl;
        std::cout << "Theta: " << greeks.theta << std::endl;
        std::cout << "Vega: " << greeks.vega << std::endl;
        
        // Monte Carlo simulation
        MonteCarloParams mc_params;
        mc_params.spot_price = params.spot_price;
        mc_params.strike_price = params.strike_price;
        mc_params.time_to_expiry = params.time_to_expiry;
        mc_params.risk_free_rate = params.risk_free_rate;
        mc_params.volatility = params.volatility;
        mc_params.is_call = true;
        mc_params.num_simulations = 50000;
        mc_params.num_steps = 63; // Quarterly steps
        
        SimulationResult mc_result = brownian_motion_.priceOption(mc_params);
        std::cout << "Monte Carlo Call Price: $" << std::setprecision(2) << mc_result.option_price 
                  << " (±$" << mc_result.standard_error << ")" << std::endl;
        std::cout << "95% CI: [$" << mc_result.confidence_interval_lower 
                  << ", $" << mc_result.confidence_interval_upper << "]" << std::endl;
    }
    
    void updateVolatilitySurface() {
        // Simulate updating volatility surface from market data
        std::cout << "Updating volatility surface..." << std::endl;
        
        // In real implementation, would fetch market data and calculate implied volatilities
        // for different strikes and expirations to build volatility surface
    }
};

int main() {
    OptionsService service;
    
    if (!service.initialize()) {
        std::cerr << "Failed to initialize Options Service" << std::endl;
        return 1;
    }
    
    service.run();
    return 0;
}