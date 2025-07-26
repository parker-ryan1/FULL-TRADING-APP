#pragma once

#include "order.h"
#include <map>
#include <set>
#include <memory>
#include <vector>
#include <mutex>

namespace hedgefund {
namespace orderbook {

struct Trade {
    std::string buy_order_id;
    std::string sell_order_id;
    double price;
    double quantity;
    std::chrono::system_clock::time_point timestamp;
};

class OrderBook {
public:
    OrderBook(const std::string& symbol);
    
    void addOrder(std::shared_ptr<Order> order);
    bool cancelOrder(const std::string& order_id);
    std::vector<Trade> matchOrders();
    
    double getBestBid() const;
    double getBestAsk() const;
    double getSpread() const;
    
    std::vector<std::pair<double, double>> getBidLevels(int depth = 10) const;
    std::vector<std::pair<double, double>> getAskLevels(int depth = 10) const;
    
private:
    std::string symbol_;
    
    // Price-time priority: price first, then time
    struct BuyComparator {
        bool operator()(const std::shared_ptr<Order>& a, const std::shared_ptr<Order>& b) const {
            if (a->price != b->price) return a->price > b->price; // Higher price first for buys
            return a->timestamp < b->timestamp; // Earlier time first
        }
    };
    
    struct SellComparator {
        bool operator()(const std::shared_ptr<Order>& a, const std::shared_ptr<Order>& b) const {
            if (a->price != b->price) return a->price < b->price; // Lower price first for sells
            return a->timestamp < b->timestamp; // Earlier time first
        }
    };
    
    std::set<std::shared_ptr<Order>, BuyComparator> buy_orders_;
    std::set<std::shared_ptr<Order>, SellComparator> sell_orders_;
    std::map<std::string, std::shared_ptr<Order>> order_map_;
    
    mutable std::mutex mutex_;
    
    std::vector<Trade> executeTrade(std::shared_ptr<Order> buy_order, std::shared_ptr<Order> sell_order, double price, double quantity);
};

} // namespace orderbook
} // namespace hedgefund