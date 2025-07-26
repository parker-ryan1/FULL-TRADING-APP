#include "orderbook.h"
#include <algorithm>
#include <iostream>

namespace hedgefund {
namespace orderbook {

OrderBook::OrderBook(const std::string& symbol) : symbol_(symbol) {}

void OrderBook::addOrder(std::shared_ptr<Order> order) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    order_map_[order->id] = order;
    
    if (order->side == OrderSide::BUY) {
        buy_orders_.insert(order);
    } else {
        sell_orders_.insert(order);
    }
    
    std::cout << "Added order: " << order->id << " " << (order->side == OrderSide::BUY ? "BUY" : "SELL") 
              << " " << order->quantity << "@" << order->price << std::endl;
}

bool OrderBook::cancelOrder(const std::string& order_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = order_map_.find(order_id);
    if (it == order_map_.end()) return false;
    
    auto order = it->second;
    order->status = OrderStatus::CANCELLED;
    
    if (order->side == OrderSide::BUY) {
        buy_orders_.erase(order);
    } else {
        sell_orders_.erase(order);
    }
    
    order_map_.erase(it);
    return true;
}

std::vector<Trade> OrderBook::matchOrders() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Trade> trades;
    
    while (!buy_orders_.empty() && !sell_orders_.empty()) {
        auto best_buy = *buy_orders_.begin();
        auto best_sell = *sell_orders_.begin();
        
        // Check if orders can match
        if (best_buy->price < best_sell->price) break;
        
        double trade_price = best_sell->price; // Price improvement for buyer
        double trade_quantity = std::min(best_buy->remainingQuantity(), best_sell->remainingQuantity());
        
        auto new_trades = executeTrade(best_buy, best_sell, trade_price, trade_quantity);
        trades.insert(trades.end(), new_trades.begin(), new_trades.end());
        
        // Remove completed orders
        if (best_buy->isComplete()) {
            buy_orders_.erase(buy_orders_.begin());
            order_map_.erase(best_buy->id);
        }
        
        if (best_sell->isComplete()) {
            sell_orders_.erase(sell_orders_.begin());
            order_map_.erase(best_sell->id);
        }
    }
    
    return trades;
}

std::vector<Trade> OrderBook::executeTrade(std::shared_ptr<Order> buy_order, std::shared_ptr<Order> sell_order, 
                                          double price, double quantity) {
    std::vector<Trade> trades;
    
    buy_order->filled_quantity += quantity;
    sell_order->filled_quantity += quantity;
    
    if (buy_order->remainingQuantity() == 0) {
        buy_order->status = OrderStatus::FILLED;
    } else {
        buy_order->status = OrderStatus::PARTIAL_FILLED;
    }
    
    if (sell_order->remainingQuantity() == 0) {
        sell_order->status = OrderStatus::FILLED;
    } else {
        sell_order->status = OrderStatus::PARTIAL_FILLED;
    }
    
    Trade trade{
        buy_order->id,
        sell_order->id,
        price,
        quantity,
        std::chrono::system_clock::now()
    };
    
    trades.push_back(trade);
    
    std::cout << "Trade executed: " << quantity << "@" << price 
              << " between " << buy_order->id << " and " << sell_order->id << std::endl;
    
    return trades;
}

double OrderBook::getBestBid() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return buy_orders_.empty() ? 0.0 : (*buy_orders_.begin())->price;
}

double OrderBook::getBestAsk() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return sell_orders_.empty() ? 0.0 : (*sell_orders_.begin())->price;
}

double OrderBook::getSpread() const {
    double bid = getBestBid();
    double ask = getBestAsk();
    return (bid > 0 && ask > 0) ? ask - bid : 0.0;
}

std::vector<std::pair<double, double>> OrderBook::getBidLevels(int depth) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::pair<double, double>> levels;
    
    std::map<double, double> price_levels;
    for (const auto& order : buy_orders_) {
        price_levels[order->price] += order->remainingQuantity();
    }
    
    int count = 0;
    for (auto it = price_levels.rbegin(); it != price_levels.rend() && count < depth; ++it, ++count) {
        levels.emplace_back(it->first, it->second);
    }
    
    return levels;
}

std::vector<std::pair<double, double>> OrderBook::getAskLevels(int depth) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::pair<double, double>> levels;
    
    std::map<double, double> price_levels;
    for (const auto& order : sell_orders_) {
        price_levels[order->price] += order->remainingQuantity();
    }
    
    int count = 0;
    for (auto it = price_levels.begin(); it != price_levels.end() && count < depth; ++it, ++count) {
        levels.emplace_back(it->first, it->second);
    }
    
    return levels;
}

} // namespace orderbook
} // namespace hedgefund