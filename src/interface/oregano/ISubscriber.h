#pragma once

#include "IMessageHandler.h"

#include <functional>
#include <string>

namespace oregano {

class ISubscriber : public IMessageHandler {
public:
    using on_message_callback = std::function<void(const std::string& p_channel, const std::string& p_message)>;

public:
    virtual ~ISubscriber() = default;

    virtual void subscribe(const std::string& p_channel) = 0;
    virtual void unsubscribe(const std::string& p_channel) = 0;

    virtual void set_on_message_callback(const on_message_callback& p_callback) = 0;
};

} // namespace oregano
