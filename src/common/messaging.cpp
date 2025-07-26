#include "common/messaging.h"
#include <iostream>
#include <chrono>

namespace hedgefund {
namespace common {

MessageQueue::MessageQueue(const std::string& broker_url) 
    : broker_url_(broker_url), running_(false) {}

MessageQueue::~MessageQueue() {
    stopConsumer();
    disconnect();
}

bool MessageQueue::connect() {
    // Simplified connection - in real implementation would use ActiveMQ Artemis C++ client
    std::cout << "Connected to message broker: " << broker_url_ << std::endl;
    return true;
}

void MessageQueue::disconnect() {
    std::cout << "Disconnected from message broker" << std::endl;
}

bool MessageQueue::publish(const std::string& topic, const std::string& payload) {
    // Simplified publish - in real implementation would use ActiveMQ Artemis
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    std::cout << "Publishing to " << topic << ": " << payload.substr(0, 100) 
              << (payload.length() > 100 ? "..." : "") << std::endl;
    
    return true;
}

bool MessageQueue::subscribe(const std::string& topic, std::function<void(const Message&)> callback) {
    // Simplified subscribe - in real implementation would register callback with ActiveMQ Artemis
    std::cout << "Subscribed to topic: " << topic << std::endl;
    callbacks_[topic] = callback;
    return true;
}

void MessageQueue::startConsumer() {
    if (running_) return;
    
    running_ = true;
    consumer_thread_ = std::make_unique<std::thread>(&MessageQueue::consumerLoop, this);
}

void MessageQueue::stopConsumer() {
    if (!running_) return;
    
    running_ = false;
    if (consumer_thread_ && consumer_thread_->joinable()) {
        consumer_thread_->join();
    }
}

void MessageQueue::consumerLoop() {
    int counter = 0;
    while (running_) {
        // Simulate receiving messages every 10 seconds
        if (counter % 100 == 0) {
            // Simulate some test messages
            for (auto& [topic, callback] : callbacks_) {
                Message msg;
                msg.topic = topic;
                msg.payload = "test_message_" + std::to_string(counter);
                msg.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                msg.correlation_id = "corr_" + std::to_string(counter);
                
                if (counter % 1000 == 0) { // Only call occasionally to avoid spam
                    callback(msg);
                }
            }
        }
        counter++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

} // namespace common
} // namespace hedgefund