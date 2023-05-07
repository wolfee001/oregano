#pragma once

#include <oregano/IMessageBrokerWrapper.h>

#include "IMessageHandlerManager.h"

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace oregano {

class MessageHandlerManager : public IMessageHandlerManager {
public:
    enum class Type {
        Event,
        Transaction,
    };

public:
    MessageHandlerManager(Type p_subscription_type, IMessageBrokerWrapper& p_message_broker_wrapper);

    virtual void add_message_handler(const std::string& p_channel, std::reference_wrapper<IMessageHandler> p_subscriber) override;
    virtual void remove_message_handler(const std::string& p_channel, std::reference_wrapper<IMessageHandler> p_subscriber) override;
    virtual void remove_message_handler(std::reference_wrapper<IMessageHandler> p_subscriber) override;
    virtual void on_message(const std::string& p_channel, const std::string& p_message) override;

private:
    Type m_subscripption_type;
    IMessageBrokerWrapper& m_message_broker_wrapper;
    std::unordered_map<std::string, std::vector<std::reference_wrapper<IMessageHandler>>> m_channel_subscriptions;
    std::unique_ptr<IMessageHandlerManager> m_message_handler_manager;
    std::mutex m_channel_subscription_lock;
};

} // namespace oregano
