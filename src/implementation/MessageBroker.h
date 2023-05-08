#pragma once

#include "IMessageBroker.h"

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

namespace oregano {

class MessageBroker : public IMessageBroker {
public:
    explicit MessageBroker(std::unique_ptr<IMessageBrokerConnector> p_connector);

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
    std::unique_ptr<IMessageBrokerConnector> m_connector;
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