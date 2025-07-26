#include "orderbook.h"
#include "common/database.h"
#include "common/messaging.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <sstream>

using namespace hedgefund::orderbook;
using namespace hedgefund::common;

class OrderBookService {
public:
    OrderBookService() 
        : db_("host=localhost port=5432 dbname=hedgefund user=trader password=secure_password"),
          mq_("tcp://localhost:61616"),
          orderbook_("AAPL") {}
    
    bool initialize() {
        if (!db_.connect()) {
            std::cerr << "Failed to connect to database" << std::endl;
            return false;
        }
        
        if (!mq_.connect()) {
            std::cerr << "Failed to connect to message queue" << std::endl;
            return false;
        }
        
        // Subscribe to order events
        mq_.subscribe("orders.new", [this](const Message& msg) {
            handleNewOrder(msg);
        });
        
        mq_.subscribe("orders.cancel", [this](const Message& msg) {
            handleCancelOrder(msg);
        });
        
        mq_.startConsumer();
        return true;
    }
    
    void run() {
        std::cout << "Order Book Service started for symbol: AAPL" << std::endl;
        
        // Simulate some initial orders for testing
        simulateOrders();
        
        while (true) {
            // Match orders periodically
            auto trades = orderbook_.matchOrders();
            
            // Process trades
            for (const auto& trade : trades) {
                processTrade(trade);
            }
            
            // Publish market data
            publishMarketData();
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
private:
    Database db_;
    MessageQueue mq_;
    OrderBook orderbook_;
    
    void handleNewOrder(const Message& msg) {
        // Parse order from message payload (simplified)
        // In real implementation, would use JSON or protobuf
        std::cout << "Received new order: " << msg.payload << std::endl;
        
        // Create and add order to book
        static int order_counter = 1;
        std::string order_id = "ORDER_" + std::to_string(order_counter++);
        
        auto order = std::make_shared<Order>(
            order_id, "AAPL", OrderType::LIMIT, OrderSide::BUY, 
            150.0, 100.0, "CLIENT_1"
        );
        
        orderbook_.addOrder(order);
    }
    
    void handleCancelOrder(const Message& msg) {
        std::cout << "Received cancel order: " << msg.payload << std::endl;
        orderbook_.cancelOrder(msg.payload);
    }
    
    void processTrade(const Trade& trade) {
        // Store trade in database
        db_.insertTrade("AAPL", trade.price, trade.quantity, "MATCHED");
        
        // Publish trade event
        std::ostringstream trade_msg;
        trade_msg << "TRADE," << trade.price << "," << trade.quantity 
                  << "," << trade.buy_order_id << "," << trade.sell_order_id;
        
        mq_.publish("trades.executed", trade_msg.str());
        
        std::cout << "Processed trade: " << trade.quantity << "@" << trade.price << std::endl;
    }
    
    void publishMarketData() {
        double bid = orderbook_.getBestBid();
        double ask = orderbook_.getBestAsk();
        double spread = orderbook_.getSpread();
        
        if (bid > 0 || ask > 0) {
            std::ostringstream market_data;
            market_data << "MARKET_DATA,AAPL," << bid << "," << ask << "," << spread;
            
            mq_.publish("market.data", market_data.str());
            
            // Store in database
            if (ask > 0) {
                auto now = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                db_.insertMarketData("AAPL", ask, 0, now);
            }
        }
    }
    
    void simulateOrders() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> price_dist(149.0, 151.0);
        std::uniform_real_distribution<> qty_dist(50.0, 200.0);
        std::uniform_int_distribution<> side_dist(0, 1);
        
        // Add some initial orders
        for (int i = 0; i < 10; i++) {
            std::string order_id = "SIM_" + std::to_string(i);
            OrderSide side = side_dist(gen) ? OrderSide::BUY : OrderSide::SELL;
            double price = price_dist(gen);
            double quantity = qty_dist(gen);
            
            auto order = std::make_shared<Order>(
                order_id, "AAPL", OrderType::LIMIT, side, price, quantity, "SIM_CLIENT"
            );
            
            orderbook_.addOrder(order);
        }
    }
};

int main() {
    OrderBookService service;
    
    if (!service.initialize()) {
        std::cerr << "Failed to initialize Order Book Service" << std::endl;
        return 1;
    }
    
    service.run();
    return 0;
}