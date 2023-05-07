#include "RedisMessageBroker.h"

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

RedisClass::RedisClass(const std::string& p_host, uint16_t p_port, const std::string& p_technical_channel)
    : m_host(p_host)
    , m_port(p_port)
    , m_technical_channel(p_technical_channel)
{
    sw::redis::ConnectionOptions opts;
    opts.host = m_host;
    opts.port = m_port;

    m_redis = std::make_unique<sw::redis::Redis>(opts);
    m_redis_queue = std::make_unique<sw::redis::Redis>(opts);
    m_redis_subscriber = std::make_unique<sw::redis::Subscriber>(m_redis->subscriber());

    m_redis_subscriber->subscribe(m_technical_channel);
    m_queue_subscriptions.push_back(m_technical_channel);
    m_queue_subscriptions_changed = true;

    m_is_running = true;

    m_redis_subscriber->on_message(
        [&m_technical_channel = this->m_technical_channel, &m_on_subscribe_callback = this->m_on_subscribe_callback](
            const std::string& p_channel, const std::string& p_message) {
            if (p_channel == m_technical_channel) {
                return;
            }

            m_on_subscribe_callback(p_channel, p_message);
        });

    m_subscriber_worker = std::thread([&m_redis_subscriber = this->m_redis_subscriber, &m_is_running = this->m_is_running]() {
        while (m_is_running) {
            m_redis_subscriber->consume();
        }
    });

    m_queue_worker = std::thread(
        [&m_redis_queue = this->m_redis_queue, &m_is_running = this->m_is_running,
            &m_queue_subscription_lock = this->m_queue_subscription_lock,
            &m_queue_subscriptions_changed = this->m_queue_subscriptions_changed, &m_queue_subscriptions = this->m_queue_subscriptions,
            &m_on_queue_callback = this->m_on_queue_callback, &m_technical_channel = this->m_technical_channel]() {
            std::list<std::string> queues;
            while (m_is_running) {
                {
                    std::lock_guard<std::mutex> guard { m_queue_subscription_lock };
                    if (m_queue_subscriptions_changed) {
                        queues = m_queue_subscriptions;
                        m_queue_subscriptions_changed = false;
                    }
                }
                const auto message = m_redis_queue->brpop(queues.begin(), queues.end());
                if (message->first == m_technical_channel) {
                    continue;
                }

                m_on_queue_callback(message->first, message->second);
            }
        });
}

RedisClass::~RedisClass()
{
    if (!m_is_running) {
        return;
    }

    m_is_running = false;
    notify_event_subscription_change();
    notify_queue_subscription_change();
    if (m_subscriber_worker.joinable()) {
        m_subscriber_worker.join();
    }
    if (m_queue_worker.joinable()) {
        m_queue_worker.join();
    }
    m_redis_subscriber->unsubscribe();
}

void RedisClass::set_on_subscriber_callback(const RedisClass::callback_t& p_on_subscribe_callback)
{
    m_on_subscribe_callback = p_on_subscribe_callback;
}

void RedisClass::set_on_queue_callback(const RedisClass::callback_t& p_on_queue_callback) { m_on_queue_callback = p_on_queue_callback; }

void RedisClass::publish(const std::string& p_channel, const std::string& p_message)
{
    std::lock_guard<std::mutex> guard { m_redis_action_lock };
    m_redis->publish(p_channel, p_message);
}

void RedisClass::subscribe(const std::string& p_channel)
{
    m_redis_subscriber->subscribe(p_channel);
    notify_event_subscription_change();
}

void RedisClass::unsubscribe(const std::string& p_channel)
{
    m_redis_subscriber->unsubscribe(p_channel);
    notify_event_subscription_change();
}

void RedisClass::send_request(const std::string& p_queue, const std::string& p_message)
{
    std::lock_guard<std::mutex> guard { m_redis_action_lock };
    m_redis->lpush(p_queue, p_message);
}

void RedisClass::listen(const std::string& p_queue)
{
    std::lock_guard<std::mutex> guard { m_queue_subscription_lock };
    m_queue_subscriptions.push_back(p_queue);
    m_queue_subscriptions_changed = true;
    notify_queue_subscription_change();
}

void RedisClass::stop_listening(const std::string& p_queue)
{
    std::lock_guard<std::mutex> guard { m_queue_subscription_lock };
    m_queue_subscriptions.remove(p_queue);
    m_queue_subscriptions_changed = true;
    notify_queue_subscription_change();
}

void RedisClass::notify_event_subscription_change()
{
    std::lock_guard<std::mutex> guard { m_redis_action_lock };
    m_redis->publish(m_technical_channel, "");
}

void RedisClass::notify_queue_subscription_change()
{
    std::lock_guard<std::mutex> guard { m_redis_action_lock };
    m_redis->rpush(m_technical_channel, "");
}

RedisMessageBroker::RedisMessageBroker(std::unique_ptr<RedisClass> p_redis_class)
    : m_redis_class(std::move(p_redis_class))
{
    m_redis_class->set_on_subscriber_callback(
        [&m_event_callbacks = this->m_event_callbacks, &m_event_callbacks_lock = this->m_event_callbacks_lock](
            const std::string& p_channel, const std::string& p_message) {
            std::lock_guard<std::mutex> guard { m_event_callbacks_lock };
            for (auto& element : m_event_callbacks) {
                element.second(p_channel, p_message);
            }
        });

    m_redis_class->set_on_queue_callback(
        [&m_transaction_callbacks_lock = this->m_transaction_callbacks_lock, &m_transaction_callbacks = this->m_transaction_callbacks](
            const std::string& p_channel, const std::string& p_message) {
            std::lock_guard<std::mutex> guard { m_transaction_callbacks_lock };
            for (auto& element : m_transaction_callbacks) {
                element.second(p_channel, p_message);
            }
        });
}

void RedisMessageBroker::init() { }

RedisMessageBroker::~RedisMessageBroker() = default;

void RedisMessageBroker::publish(const std::string& p_channel, const std::string& p_message)
{
    m_redis_class->publish(p_channel, p_message);
}

void RedisMessageBroker::subscribe(const std::string& p_channel)
{
    std::lock_guard<std::mutex> guard { m_channel_subscription_lock };
    m_channel_subscriptions[p_channel]++;

    if (m_channel_subscriptions[p_channel] == 1) {
        m_redis_class->subscribe(p_channel);
    }
}

void RedisMessageBroker::unsubscribe(const std::string& p_channel)
{
    std::lock_guard<std::mutex> guard { m_channel_subscription_lock };
    m_channel_subscriptions[p_channel]--;

    if (m_channel_subscriptions[p_channel] == 0) {
        m_channel_subscriptions.erase(p_channel);
        m_redis_class->unsubscribe(p_channel);
    }
}

std::string RedisMessageBroker::register_event_callback(const on_message_callback& p_callback)
{
    std::lock_guard<std::mutex> guard { m_event_callbacks_lock };
    std::string id = sole::uuid4().base62();
    m_event_callbacks.emplace_back(id, p_callback);
    return id;
}

void RedisMessageBroker::remove_event_callback(const std::string& p_id)
{
    std::lock_guard<std::mutex> guard { m_event_callbacks_lock };
    m_event_callbacks.remove_if([&p_id](const auto& element) { return element.first == p_id; });
}

void RedisMessageBroker::send_request(const std::string& p_queue, const std::string& p_message)
{
    m_redis_class->send_request(p_queue, p_message);
}

void RedisMessageBroker::listen(const std::string& p_queue)
{
    std::lock_guard<std::mutex> guard { m_queue_subscription_lock };
    m_queue_subscriptions[p_queue]++;

    if (m_queue_subscriptions[p_queue] == 1) {
        m_redis_class->listen(p_queue);
    } else {
        throw std::runtime_error("Only one message handler should listen a given command channel!");
    }
}

void RedisMessageBroker::stop_listening(const std::string& p_queue)
{
    std::lock_guard<std::mutex> guard { m_queue_subscription_lock };
    m_queue_subscriptions[p_queue]--;

    if (m_queue_subscriptions[p_queue] == 0) {
        m_queue_subscriptions.erase(p_queue);
        m_redis_class->stop_listening(p_queue);
    }
}

std::string RedisMessageBroker::register_transaction_callback(const on_message_callback& p_callback)
{
    std::lock_guard<std::mutex> guard { m_transaction_callbacks_lock };
    std::string id = sole::uuid4().base62();
    m_transaction_callbacks.emplace_back(id, p_callback);
    return id;
}

void RedisMessageBroker::remove_transaction_callback(const std::string& p_id)
{
    std::lock_guard<std::mutex> guard { m_transaction_callbacks_lock };
    m_transaction_callbacks.remove_if([&p_id](const auto& element) { return element.first == p_id; });
}

} // namespace oregano