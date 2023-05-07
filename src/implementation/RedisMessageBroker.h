#pragma once

#include "IMessageBroker.h"

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace sw {
namespace redis {
    class Redis;
    class Subscriber;
} // namespace redis
} // namespace sw

namespace oregano {

class RedisClass {
public:
    using callback_t = std::function<void(const std::string& p_channel, const std::string& p_message)>;

    RedisClass(const std::string& p_host, uint16_t p_port, const std::string& p_technical_channel);
    ~RedisClass();

    void set_on_subscriber_callback(const callback_t& p_on_subscribe_callback);
    void set_on_queue_callback(const callback_t& p_on_queue_callback);

    void publish(const std::string& p_channel, const std::string& p_message);
    void subscribe(const std::string& p_channel);
    void unsubscribe(const std::string& p_channel);

    void send_request(const std::string& p_queue, const std::string& p_message);
    void listen(const std::string& p_queue);
    void stop_listening(const std::string& p_queue);

private:
    void notify_event_subscription_change();
    void notify_queue_subscription_change();

private:
    std::string m_host;
    uint16_t m_port { 0 };
    std::string m_technical_channel;
    std::unique_ptr<sw::redis::Redis> m_redis;
    std::unique_ptr<sw::redis::Subscriber> m_redis_subscriber;
    std::unique_ptr<sw::redis::Redis> m_redis_queue;
    std::mutex m_redis_action_lock;
    std::atomic_bool m_is_running { false };
    std::thread m_subscriber_worker;
    std::thread m_queue_worker;
    callback_t m_on_subscribe_callback = [](const std::string&, const std::string&) {};
    callback_t m_on_queue_callback = [](const std::string&, const std::string&) {};
    std::mutex m_queue_subscription_lock;
    std::list<std::string> m_queue_subscriptions;
    std::atomic_bool m_queue_subscriptions_changed;
};

class RedisMessageBroker : public IMessageBroker {
public:
    RedisMessageBroker(std::unique_ptr<RedisClass> p_redis_class);
    ~RedisMessageBroker() override;

    void init() override;

    void publish(const std::string& p_channel, const std::string& p_message) override;
    void subscribe(const std::string& p_channel) override;
    void unsubscribe(const std::string& p_channel) override;
    std::string register_event_callback(const on_message_callback& p_callback) override;
    void remove_event_callback(const std::string& p_id) override;

    void send_request(const std::string& p_queue, const std::string& p_message) override;
    void listen(const std::string& p_queue) override;
    void stop_listening(const std::string& p_queue) override;
    std::string register_transaction_callback(const on_message_callback& p_callback) override;
    void remove_transaction_callback(const std::string& p_id) override;

private:
    std::unique_ptr<RedisClass> m_redis_class;
    std::mutex m_channel_subscription_lock;
    std::unordered_map<std::string, size_t> m_channel_subscriptions;
    std::unordered_map<std::string, size_t> m_queue_subscriptions;
    std::list<std::pair<std::string, on_message_callback>> m_event_callbacks;
    std::list<std::pair<std::string, on_message_callback>> m_transaction_callbacks;
    std::mutex m_event_callbacks_lock;
    std::mutex m_transaction_callbacks_lock;
    std::mutex m_queue_subscription_lock;
};

} // namespace oregano