#include "order.h"

namespace hedgefund {
namespace orderbook {

Order::Order(const std::string& id, const std::string& symbol, OrderType type, 
             OrderSide side, double price, double quantity, const std::string& client_id)
    : id(id), symbol(symbol), type(type), side(side), price(price), 
      quantity(quantity), filled_quantity(0.0), status(OrderStatus::PENDING),
      timestamp(std::chrono::system_clock::now()), client_id(client_id) {}

double Order::remainingQuantity() const {
    return quantity - filled_quantity;
}

bool Order::isComplete() const {
    return status == OrderStatus::FILLED || status == OrderStatus::CANCELLED || status == OrderStatus::REJECTED;
}

} // namespace orderbook
} // namespace hedgefund