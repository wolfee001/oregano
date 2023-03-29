#pragma once

#include <oregano/IMessageHandler.h>

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace oregano {

class MessageHandlerManagerException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class IMessageHandlerManager {
public:
    virtual ~IMessageHandlerManager() = default;

    virtual void add_message_handler(const std::string& p_channel, std::reference_wrapper<IMessageHandler> p_subscriber) = 0;
    virtual void remove_message_handler(const std::string& p_channel, std::reference_wrapper<IMessageHandler> p_subscriber) = 0;
    virtual void remove_message_handler(std::reference_wrapper<IMessageHandler> p_subscriber) = 0;
    virtual void on_message(const std::string& p_channel, const std::string& p_message) = 0;
};

} // namespace oregano
