#pragma once

#include <string>
#include <chrono>

namespace hedgefund {
namespace orderbook {

enum class OrderType {
    MARKET,
    LIMIT,
    STOP,
    STOP_LIMIT
};

enum class OrderSide {
    BUY,
    SELL
};

enum class OrderStatus {
    PENDING,
    PARTIAL_FILLED,
    FILLED,
    CANCELLED,
    REJECTED
};

struct Order {
    std::string id;
    std::string symbol;
    OrderType type;
    OrderSide side;
    double price;
    double quantity;
    double filled_quantity;
    OrderStatus status;
    std::chrono::system_clock::time_point timestamp;
    std::string client_id;
    
    Order(const std::string& id, const std::string& symbol, OrderType type, 
          OrderSide side, double price, double quantity, const std::string& client_id);
    
    double remainingQuantity() const;
    bool isComplete() const;
};

} // namespace orderbook
} // namespace hedgefund