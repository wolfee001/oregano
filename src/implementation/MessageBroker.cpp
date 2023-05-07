#include "MessageBroker.h"

#include <sw/redis++/errors.h>
#include <sw/redis++/redis.h>
#include <sw/redis++/subscriber.h>

#include <algorithm>
#include <memory>
#include <mutex>
#include <string>

#include <sole.hpp>

using namespace std::chrono_literals;

namespace oregano {

MessageBroker::MessageBroker(std::unique_ptr<IMessageBrokerConnector> p_connector)
    : m_connector(std::move(p_connector))
{
    m_connector->set_on_subscriber_callback(
        [&m_event_callbacks = this->m_event_callbacks, &m_event_callbacks_lock = this->m_event_callbacks_lock](
            const std::string& p_channel, const std::string& p_message) {
            std::lock_guard<std::mutex> guard { m_event_callbacks_lock };
            for (auto& element : m_event_callbacks) {
                element.second(p_channel, p_message);
            }
        });

    m_connector->set_on_queue_callback(
        [&m_transaction_callbacks_lock = this->m_transaction_callbacks_lock, &m_transaction_callbacks = this->m_transaction_callbacks](
            const std::string& p_channel, const std::string& p_message) {
            std::lock_guard<std::mutex> guard { m_transaction_callbacks_lock };
            for (auto& element : m_transaction_callbacks) {
                element.second(p_channel, p_message);
            }
        });
}

void MessageBroker::publish(const std::string& p_channel, const std::string& p_message) { m_connector->publish(p_channel, p_message); }

void MessageBroker::subscribe(const std::string& p_channel)
{
    std::lock_guard<std::mutex> guard { m_channel_subscription_lock };
    m_channel_subscriptions[p_channel]++;

    if (m_channel_subscriptions[p_channel] == 1) {
        m_connector->subscribe(p_channel);
    }
}

void MessageBroker::unsubscribe(const std::string& p_channel)
{
    std::lock_guard<std::mutex> guard { m_channel_subscription_lock };
    m_channel_subscriptions[p_channel]--;

    if (m_channel_subscriptions[p_channel] == 0) {
        m_channel_subscriptions.erase(p_channel);
        m_connector->unsubscribe(p_channel);
    }
}

std::string MessageBroker::register_event_callback(const on_message_callback& p_callback)
{
    std::lock_guard<std::mutex> guard { m_event_callbacks_lock };
    std::string id = sole::uuid4().base62();
    m_event_callbacks.emplace_back(id, p_callback);
    return id;
}

void MessageBroker::remove_event_callback(const std::string& p_id)
{
    std::lock_guard<std::mutex> guard { m_event_callbacks_lock };
    m_event_callbacks.remove_if([&p_id](const auto& element) { return element.first == p_id; });
}

void MessageBroker::send_request(const std::string& p_queue, const std::string& p_message)
{
    m_connector->send_request(p_queue, p_message);
}

void MessageBroker::listen(const std::string& p_queue)
{
    std::lock_guard<std::mutex> guard { m_queue_subscription_lock };
    m_queue_subscriptions[p_queue]++;

    if (m_queue_subscriptions[p_queue] == 1) {
        m_connector->listen(p_queue);
    } else {
        throw std::runtime_error("Only one message handler should listen a given command channel!");
    }
}

void MessageBroker::stop_listening(const std::string& p_queue)
{
    std::lock_guard<std::mutex> guard { m_queue_subscription_lock };
    m_queue_subscriptions[p_queue]--;

    if (m_queue_subscriptions[p_queue] == 0) {
        m_queue_subscriptions.erase(p_queue);
        m_connector->stop_listening(p_queue);
    }
}

std::string MessageBroker::register_transaction_callback(const on_message_callback& p_callback)
{
    std::lock_guard<std::mutex> guard { m_transaction_callbacks_lock };
    std::string id = sole::uuid4().base62();
    m_transaction_callbacks.emplace_back(id, p_callback);
    return id;
}

void MessageBroker::remove_transaction_callback(const std::string& p_id)
{
    std::lock_guard<std::mutex> guard { m_transaction_callbacks_lock };
    m_transaction_callbacks.remove_if([&p_id](const auto& element) { return element.first == p_id; });
}

} // namespace oregano