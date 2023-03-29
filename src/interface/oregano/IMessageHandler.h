#pragma once

#include <string>

namespace oregano {

class IMessageHandler {
public:
    virtual ~IMessageHandler() = default;

    virtual void on_message(const std::string& p_channel, const std::string& p_message) = 0;
};

} // namespace oregano
