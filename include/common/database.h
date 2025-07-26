#pragma once

#include <string>
#include <vector>
#include <memory>

namespace hedgefund {
namespace common {

// Simplified database interface for Windows compilation
class Database {
public:
    Database(const std::string& connection_string);
    ~Database();
    
    bool connect();
    void disconnect();
    bool execute(const std::string& query);
    
    // Time-series specific methods
    bool insertMarketData(const std::string& symbol, double price, double volume, int64_t timestamp);
    bool insertTrade(const std::string& symbol, double price, double quantity, const std::string& side);
    std::vector<std::pair<int64_t, double>> getPriceHistory(const std::string& symbol, int64_t start_time, int64_t end_time);
    
private:
    std::string connection_string_;
    bool connected_;
};

} // namespace common
} // namespace hedgefund