#pragma once

#include <string>

namespace oregano {

class IPublisher {
public:
    virtual ~IPublisher() = default;

    virtual void publish(const std::string& p_channel, const std::string& p_message) = 0;
};

} // namespace oregano