#pragma once

#include "IMessageBrokerWrapper.h"
#include "IPublisher.h"
#include "ISubscriber.h"

#include <memory>

namespace oregano {

class IEventInterface {
public:
    virtual ~IEventInterface() = default;

    virtual std::unique_ptr<IPublisher> create_publisher() = 0;
    virtual std::unique_ptr<ISubscriber> create_subscriber() = 0;

    static std::unique_ptr<IEventInterface> create(IMessageBrokerWrapper& p_message_broker);
};

} // namespace oregano