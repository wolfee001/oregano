#pragma once

#include "ISubscriber.h"

#include "BrokerConfiguration.h"

#include <functional>
#include <memory>

namespace oregano {

class IMessageBroker;

class IMessageBrokerWrapper {
public:
    virtual ~IMessageBrokerWrapper() = default;

    virtual IMessageBroker& get_message_broker() = 0;

    static std::unique_ptr<IMessageBrokerWrapper> create(broker_configuration::Configuration p_configuration);
};

} // namespace oregano