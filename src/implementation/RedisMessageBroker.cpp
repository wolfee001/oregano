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

RedisMessageBroker::RedisMessageBroker(const std::string& p_host, uint16_t p_port, const std::string& p_technical_channel)
    : m_is_running(true)
    , m_technical_channel(p_technical_channel)
{
    sw::redis::ConnectionOptions opts;
    opts.host = p_host;
    opts.port = p_port;

    m_redis = std::make_unique<sw::redis::Redis>(opts);
    m_redis_queue = std::make_unique<sw::redis::Redis>(opts);
    m_redis_subscriber = std::make_unique<sw::redis::Subscriber>(m_redis->subscriber());

    m_redis_subscriber->subscribe(m_technical_channel);

    m_redis_subscriber->on_message(
        [&m_event_callbacks = this->m_event_callbacks, &m_event_callbacks_lock = this->m_event_callbacks_lock,
            &m_technical_channel = this->m_technical_channel](const std::string& p_channel, const std::string& p_message) {
            if (p_channel == m_technical_channel) {
                return;
            }

            std::lock_guard<std::mutex> guard { m_event_callbacks_lock };
            for (auto& element : m_event_callbacks) {
                element.second(p_channel, p_message);
            }
        });

    m_subscriber_worker = std::thread([&m_redis_subscriber = this->m_redis_subscriber, &m_is_running = this->m_is_running]() {
        while (m_is_running) {
            m_redis_subscriber->consume();
        }
    });

    m_queue_worker = std::thread(
        [&m_redis_queue = this->m_redis_queue, &m_is_running = this->m_is_running,
            &m_queue_subscription_lock = this->m_queue_subscription_lock,
            &m_transaction_callbacks_lock = this->m_transaction_callbacks_lock, &m_transaction_callbacks = this->m_transaction_callbacks,
            &m_queue_subscriptions = this->m_queue_subscriptions, &m_technical_channel = this->m_technical_channel]() {
            while (m_is_running) {
                std::list<std::string> queues { m_technical_channel };
                {
                    std::lock_guard<std::mutex> guard { m_queue_subscription_lock };
                    for (const auto& element : m_queue_subscriptions) {
                        queues.push_back(element.first);
                    }
                }
                const auto message = m_redis_queue->brpop(queues.begin(), queues.end());
                if (message->first == m_technical_channel) {
                    continue;
                }

                {
                    std::lock_guard<std::mutex> guard { m_transaction_callbacks_lock };
                    for (auto& element : m_transaction_callbacks) {
                        element.second(message->first, message->second);
                    }
                }
            }
        });
}

RedisMessageBroker::~RedisMessageBroker()
{
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

void RedisMessageBroker::publish(const std::string& p_channel, const std::string& p_message)
{
    std::lock_guard<std::mutex> guard { m_redis_action_lock };
    m_redis->publish(p_channel, p_message);
}

void RedisMessageBroker::subscribe(const std::string& p_channel)
{
    std::lock_guard<std::mutex> guard { m_channel_subscription_lock };
    m_channel_subscriptions[p_channel]++;

    if (m_channel_subscriptions[p_channel] == 1) {
        m_redis_subscriber->subscribe(p_channel);
        notify_event_subscription_change();
    }
}

void RedisMessageBroker::unsubscribe(const std::string& p_channel)
{
    std::lock_guard<std::mutex> guard { m_channel_subscription_lock };
    m_channel_subscriptions[p_channel]--;

    if (m_channel_subscriptions[p_channel] == 0) {
        m_channel_subscriptions.erase(p_channel);
        m_redis_subscriber->unsubscribe(p_channel);
        notify_event_subscription_change();
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
    std::lock_guard<std::mutex> guard { m_redis_action_lock };
    m_redis->lpush(p_queue, p_message);
}

void RedisMessageBroker::listen(const std::string& p_queue)
{
    std::lock_guard<std::mutex> guard { m_queue_subscription_lock };
    m_queue_subscriptions[p_queue]++;

    if (m_queue_subscriptions[p_queue] == 1) {
        notify_queue_subscription_change();
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
        notify_queue_subscription_change();
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

void RedisMessageBroker::notify_event_subscription_change()
{
    std::lock_guard<std::mutex> guard { m_redis_action_lock };
    m_redis->publish(m_technical_channel, "");
}

void RedisMessageBroker::notify_queue_subscription_change()
{
    std::lock_guard<std::mutex> guard { m_redis_action_lock };
    m_redis->rpush(m_technical_channel, "");
}

} // namespace oregano