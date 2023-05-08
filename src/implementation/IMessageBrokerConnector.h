#pragma once

#include <functional>
#include <string>

namespace oregano {

class IMessageBrokerConnector {
public:
    using callback_t = std::function<void(const std::string& p_channel, const std::string& p_message)>;

    virtual ~IMessageBrokerConnector() = default;

    virtual void set_on_subscriber_callback(const callback_t& p_on_subscribe_callback) = 0;
    virtual void set_on_queue_callback(const callback_t& p_on_queue_callback) = 0;

    virtual void publish(const std::string& p_channel, const std::string& p_message) = 0;
    virtual void subscribe(const std::string& p_channel) = 0;
    virtual void unsubscribe(const std::string& p_channel) = 0;

    virtual void send_request(const std::string& p_queue, const std::string& p_message) = 0;
    virtual void listen(const std::string& p_queue) = 0;
    virtual void stop_listening(const std::string& p_queue) = 0;
};

} // namespace oregano