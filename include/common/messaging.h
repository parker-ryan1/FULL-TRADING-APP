#pragma once

#include <string>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include <unordered_map>

namespace hedgefund {
namespace common {

struct Message {
    std::string topic;
    std::string payload;
    int64_t timestamp;
    std::string correlation_id;
};

class MessageQueue {
public:
    MessageQueue(const std::string& broker_url);
    ~MessageQueue();
    
    bool connect();
    void disconnect();
    
    bool publish(const std::string& topic, const std::string& payload);
    bool subscribe(const std::string& topic, std::function<void(const Message&)> callback);
    
    void startConsumer();
    void stopConsumer();
    
private:
    std::string broker_url_;
    std::atomic<bool> running_;
    std::unique_ptr<std::thread> consumer_thread_;
    std::unordered_map<std::string, std::function<void(const Message&)>> callbacks_;
    
    void consumerLoop();
};

} // namespace common
} // namespace hedgefund