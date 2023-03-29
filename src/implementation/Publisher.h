#pragma once

#include <oregano/IPublisher.h>

#include "IMessageBroker.h"

namespace oregano {

class Publisher : public IPublisher {
public:
    explicit Publisher(IMessageBroker& p_message_broker);

    void publish(const std::string& p_channel, const std::string& p_message) override;

private:
    IMessageBroker& m_message_broker;
};

} // namespace oregano