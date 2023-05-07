#pragma once

#include "IMessageBrokerConnector.h"

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

class RedisBrokerConnector : public IMessageBrokerConnector {
public:
    RedisBrokerConnector(const std::string& p_host, uint16_t p_port, const std::string& p_technical_channel);
    ~RedisBrokerConnector() override;

    void set_on_subscriber_callback(const callback_t& p_on_subscribe_callback) override;
    void set_on_queue_callback(const callback_t& p_on_queue_callback) override;

    void publish(const std::string& p_channel, const std::string& p_message) override;
    void subscribe(const std::string& p_channel) override;
    void unsubscribe(const std::string& p_channel) override;

    void send_request(const std::string& p_queue, const std::string& p_message) override;
    void listen(const std::string& p_queue) override;
    void stop_listening(const std::string& p_queue) override;

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
    std::atomic_bool m_subscriber_thread_alive { false };
    std::atomic_bool m_queue_thread_alive { false };
    std::mutex m_subscription_lock;
    std::atomic_bool m_subscription_status_changed { false };
};

} // namespace oregano