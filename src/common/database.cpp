#include "common/database.h"
#include <iostream>
#include <sstream>

namespace hedgefund {
namespace common {

Database::Database(const std::string& connection_string) 
    : connection_string_(connection_string), connected_(false) {}

Database::~Database() {
    disconnect();
}

bool Database::connect() {
    // Simplified connection for demo - in production would use actual PostgreSQL client
    std::cout << "Connecting to database: " << connection_string_ << std::endl;
    connected_ = true;
    return true;
}

void Database::disconnect() {
    if (connected_) {
        std::cout << "Disconnected from database" << std::endl;
        connected_ = false;
    }
}

bool Database::execute(const std::string& query) {
    if (!connected_) return false;
    
    std::cout << "Executing query: " << query.substr(0, 100) 
              << (query.length() > 100 ? "..." : "") << std::endl;
    return true;
}

bool Database::insertMarketData(const std::string& symbol, double price, double volume, int64_t timestamp) {
    std::ostringstream query;
    query << "INSERT INTO market_data (symbol, price, volume, timestamp) VALUES ('"
          << symbol << "', " << price << ", " << volume << ", to_timestamp(" << timestamp << "))";
    
    return execute(query.str());
}

bool Database::insertTrade(const std::string& symbol, double price, double quantity, const std::string& side) {
    std::ostringstream query;
    query << "INSERT INTO trades (symbol, price, quantity, side, timestamp) VALUES ('"
          << symbol << "', " << price << ", " << quantity << ", '" << side << "', NOW())";
    
    return execute(query.str());
}

std::vector<std::pair<int64_t, double>> Database::getPriceHistory(const std::string& symbol, int64_t start_time, int64_t end_time) {
    std::vector<std::pair<int64_t, double>> history;
    
    std::cout << "Getting price history for " << symbol << " from " << start_time << " to " << end_time << std::endl;
    
    // Simulate some price data
    for (int i = 0; i < 10; i++) {
        int64_t timestamp = start_time + i * 3600; // hourly data
        double price = 150.0 + (i * 0.5); // simulate price movement
        history.emplace_back(timestamp, price);
    }
    
    return history;
}

} // namespace common
} // namespace hedgefund