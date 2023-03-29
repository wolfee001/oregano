#pragma once

#include <oregano/IMessageBrokerWrapper.h>

#include "IMessageBroker.h"

namespace oregano {

class MessageBrokerWrapper : public IMessageBrokerWrapper {
public:
    explicit MessageBrokerWrapper(std::unique_ptr<IMessageBroker> p_message_broker);

    IMessageBroker& get_message_broker() override;

private:
    std::unique_ptr<IMessageBroker> m_message_broker;
};

} // namespace oregano