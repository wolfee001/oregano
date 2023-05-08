#pragma once

#include "ISubscriber.h"

#include "BrokerConfiguration.h"

#include <functional>
#include <memory>

namespace oregano {

class IMessageBroker;

/**
 * A wrapper class over a message broker to hide the actual interface of the broker
 */
class IMessageBrokerWrapper {
public:
    virtual ~IMessageBrokerWrapper() = default;

    /**
     * Returns the actual wrapped broker
     *
     * @return the wrapped broker
     */
    virtual IMessageBroker& get_message_broker() = 0;

    /**
     * Creates an actual implementation
     *
     * @param p_configuration a broker configuration
     * @return an actual implementation
     */
    static std::unique_ptr<IMessageBrokerWrapper> create(broker_configuration::Configuration p_configuration);
};

} // namespace oregano