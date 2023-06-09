#include "RedisBrokerConnector.h"

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

RedisBrokerConnector::RedisBrokerConnector(
    std::unique_ptr<broker_configuration::Redis> p_configuration, const std::string& p_technical_channel)
    : m_configuration(std::move(p_configuration))
    , m_technical_channel(p_technical_channel)
{
    sw::redis::ConnectionOptions opts;
    opts.host = m_configuration->get_host();
    opts.port = m_configuration->get_port();
    opts.user = m_configuration->get_user();
    opts.password = m_configuration->get_password();

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

    m_redis_subscriber->on_meta([&m_subscription_status_changed = this->m_subscription_status_changed](
                                    sw::redis::Subscriber::MsgType p_type, sw::redis::OptionalString, long long) {
        if (p_type == sw::redis::Subscriber::MsgType::SUBSCRIBE || p_type == sw::redis::Subscriber::MsgType::UNSUBSCRIBE) {
            if (m_subscription_status_changed) {
                throw std::runtime_error("subscriptions / unsunscriptions must be mutexed!");
            }
            m_subscription_status_changed = true;
        }
    });

    m_subscriber_worker = std::thread([&m_redis_subscriber = this->m_redis_subscriber, &m_is_running = this->m_is_running,
                                          &m_subscriber_thread_alive = this->m_subscriber_thread_alive]() {
        while (m_is_running) {
            m_redis_subscriber->consume();
            m_subscriber_thread_alive = true;
        }
    });

    m_queue_worker
        = std::thread([&m_redis_queue = this->m_redis_queue, &m_is_running = this->m_is_running,
                          &m_queue_subscription_lock = this->m_queue_subscription_lock,
                          &m_queue_subscriptions_changed = this->m_queue_subscriptions_changed,
                          &m_queue_subscriptions = this->m_queue_subscriptions, &m_on_queue_callback = this->m_on_queue_callback,
                          &m_technical_channel = this->m_technical_channel, &m_queue_thread_alive = this->m_queue_thread_alive]() {
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
                  m_queue_thread_alive = true;
                  if (message->first == m_technical_channel) {
                      continue;
                  }

                  m_on_queue_callback(message->first, message->second);
              }
          });

    while (!m_subscriber_thread_alive) {
        notify_event_subscription_change();
    }
    while (!m_queue_thread_alive) {
        notify_queue_subscription_change();
    }

    m_subscription_status_changed = false;
}

RedisBrokerConnector::~RedisBrokerConnector()
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

void RedisBrokerConnector::set_on_subscriber_callback(const IMessageBrokerConnector::callback_t& p_on_subscribe_callback)
{
    m_on_subscribe_callback = p_on_subscribe_callback;
}

void RedisBrokerConnector::set_on_queue_callback(const IMessageBrokerConnector::callback_t& p_on_queue_callback)
{
    m_on_queue_callback = p_on_queue_callback;
}

void RedisBrokerConnector::publish(const std::string& p_channel, const std::string& p_message)
{
    std::lock_guard<std::mutex> guard { m_redis_action_lock };
    m_redis->publish(p_channel, p_message);
}

void RedisBrokerConnector::subscribe(const std::string& p_channel)
{
    std::lock_guard<std::mutex> guard { m_subscription_lock };
    if (m_subscription_status_changed) {
        throw std::runtime_error("subscriptions / unsunscriptions must be mutexed!");
    }
    m_redis_subscriber->subscribe(p_channel);
    notify_event_subscription_change();

    while (!m_subscription_status_changed)
        ;
    m_subscription_status_changed = false;
}

void RedisBrokerConnector::unsubscribe(const std::string& p_channel)
{
    std::lock_guard<std::mutex> guard { m_subscription_lock };
    if (m_subscription_status_changed) {
        throw std::runtime_error("subscriptions / unsunscriptions must be mutexed!");
    }
    m_redis_subscriber->unsubscribe(p_channel);
    notify_event_subscription_change();

    while (!m_subscription_status_changed)
        ;
    m_subscription_status_changed = false;
}

void RedisBrokerConnector::send_request(const std::string& p_queue, const std::string& p_message)
{
    std::lock_guard<std::mutex> guard { m_redis_action_lock };
    m_redis->lpush(p_queue, p_message);
}

void RedisBrokerConnector::listen(const std::string& p_queue)
{
    std::lock_guard<std::mutex> guard { m_queue_subscription_lock };
    m_queue_subscriptions.push_back(p_queue);
    m_queue_subscriptions_changed = true;
    notify_queue_subscription_change();
}

void RedisBrokerConnector::stop_listening(const std::string& p_queue)
{
    std::lock_guard<std::mutex> guard { m_queue_subscription_lock };
    m_queue_subscriptions.remove(p_queue);
    m_queue_subscriptions_changed = true;
    notify_queue_subscription_change();
}

void RedisBrokerConnector::notify_event_subscription_change()
{
    std::lock_guard<std::mutex> guard { m_redis_action_lock };
    m_redis->publish(m_technical_channel, "");
}

void RedisBrokerConnector::notify_queue_subscription_change()
{
    std::lock_guard<std::mutex> guard { m_redis_action_lock };
    m_redis->rpush(m_technical_channel, "");
}

} // namespace oregano