#pragma once

#include <oregano/BrokerConfiguration.h>

#include <functional>
#include <memory>

namespace oregano {

class IMessageBroker {
public:
    using on_message_callback = std::function<void(const std::string& p_channel, const std::string& p_message)>;

public:
    virtual ~IMessageBroker() = default;

    virtual void publish(const std::string& p_channel, const std::string& p_message) = 0;
    virtual void subscribe(const std::string& p_channel) = 0;
    virtual void unsubscribe(const std::string& p_channel) = 0;
    virtual std::string register_event_callback(const on_message_callback& p_callback) = 0;
    virtual void remove_event_callback(const std::string& p_id) = 0;

    virtual void send_request(const std::string& p_queue, const std::string& p_message) = 0;
    virtual void listen(const std::string& p_queue) = 0;
    virtual void stop_listening(const std::string& p_queue) = 0;
    virtual std::string register_transaction_callback(const on_message_callback& p_callback) = 0;
    virtual void remove_transaction_callback(const std::string& p_id) = 0;

    static std::unique_ptr<IMessageBroker> create(broker_configuration::Configuration p_configuration);
};

} // namespace oregano