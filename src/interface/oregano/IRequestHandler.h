#pragma once

#include "IMessageHandler.h"

#include <functional>
#include <string>

namespace oregano {

class IRequestHandler : public IMessageHandler {
public:
    using on_request_callback = std::function<std::string(const std::string& p_channel, const std::string& p_message)>;

public:
    virtual ~IRequestHandler() = default;

    virtual void listen(const std::string& p_channel) = 0;
    virtual void stop_listening(const std::string& p_channel) = 0;

    virtual void set_on_request_callback(const on_request_callback& p_callback) = 0;
};

} // namespace oregano
