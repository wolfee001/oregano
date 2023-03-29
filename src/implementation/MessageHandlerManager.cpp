#include "MessageHandlerManager.h"

#include "IMessageBroker.h"

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace oregano {

MessageHandlerManager::MessageHandlerManager(Type p_subscription_type, IMessageBrokerWrapper& p_message_broker_wrapper)
    : m_subscripption_type(p_subscription_type)
    , m_message_broker_wrapper(p_message_broker_wrapper)
{
}

void MessageHandlerManager::add_message_handler(const std::string& p_channel, std::reference_wrapper<IMessageHandler> p_subscriber)
{
    std::lock_guard<std::mutex> guard { m_channel_subscription_lock };

    auto insertion_result = m_channel_subscriptions.emplace(p_channel, std::vector<std::reference_wrapper<IMessageHandler>>());
    const auto is_new_channel = insertion_result.second;

    if (!is_new_channel && m_subscripption_type == Type::Transaction) {
        throw MessageHandlerManagerException("Only one handler should listen on a transaction channel");
    }

    auto& subscriber_vector = insertion_result.first->second;
    if (std::find_if(subscriber_vector.begin(), subscriber_vector.end(),
            [&p_subscriber](const auto& element) { return &(element.get()) == &(p_subscriber.get()); })
        == subscriber_vector.end()) {
        subscriber_vector.push_back(p_subscriber);
    }

    if (is_new_channel) {
        switch (m_subscripption_type) {
        case Type::Event:
            return m_message_broker_wrapper.get_message_broker().subscribe(p_channel);
        case Type::Transaction:
            return m_message_broker_wrapper.get_message_broker().listen(p_channel);
        default:
            throw MessageHandlerManagerException("Unhandled type!");
        }
    }
}

void MessageHandlerManager::remove_message_handler(const std::string& p_channel, std::reference_wrapper<IMessageHandler> p_subscriber)
{
    std::lock_guard<std::mutex> guard { m_channel_subscription_lock };

    auto it = m_channel_subscriptions.find(p_channel);

    if (it == m_channel_subscriptions.end()) {
        return;
    }

    auto& subscribers = it->second;
    subscribers.erase(std::remove_if(subscribers.begin(), subscribers.end(),
                          [&p_subscriber](const auto& element) { return &(element.get()) == &(p_subscriber.get()); }),
        subscribers.end());

    if (!subscribers.empty()) {
        return;
    }

    m_channel_subscriptions.erase(it);

    switch (m_subscripption_type) {
    case Type::Event:
        return m_message_broker_wrapper.get_message_broker().unsubscribe(p_channel);
    case Type::Transaction:
        return m_message_broker_wrapper.get_message_broker().stop_listening(p_channel);
    default:
        throw MessageHandlerManagerException("Unhandled type!");
    }
}

void MessageHandlerManager::remove_message_handler(std::reference_wrapper<IMessageHandler> p_subscriber)
{
    std::lock_guard<std::mutex> guard { m_channel_subscription_lock };

    std::vector<std::string> empty_channels;

    for (auto channel_subscription = m_channel_subscriptions.begin(); channel_subscription != m_channel_subscriptions.end();) {
        auto& subscribers = channel_subscription->second;
        subscribers.erase(std::remove_if(subscribers.begin(), subscribers.end(),
                              [&p_subscriber](const auto& element) { return &(element.get()) == &(p_subscriber.get()); }),
            subscribers.end());

        if (subscribers.empty()) {
            empty_channels.push_back(channel_subscription->first);
            channel_subscription = m_channel_subscriptions.erase(channel_subscription);
        } else
            ++channel_subscription;
    }

    for (const auto& element : empty_channels) {
        switch (m_subscripption_type) {
        case Type::Event:
            m_message_broker_wrapper.get_message_broker().unsubscribe(element);
            break;
        case Type::Transaction:
            m_message_broker_wrapper.get_message_broker().stop_listening(element);
            break;
        default:
            throw MessageHandlerManagerException("Unhandled type!");
        }
    }
}

void MessageHandlerManager::on_message(const std::string& p_channel, const std::string& p_message)
{
    std::lock_guard<std::mutex> guard { m_channel_subscription_lock };

    if (const auto channel_subscription = m_channel_subscriptions.find(p_channel); channel_subscription != m_channel_subscriptions.end()) {
        for (const auto& subscriber : channel_subscription->second) {
            subscriber.get().on_message(p_channel, p_message);
        }
    }
}

} // namespace oregano
