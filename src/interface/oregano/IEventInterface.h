#pragma once

#include "IMessageBrokerWrapper.h"
#include "IPublisher.h"
#include "ISubscriber.h"

#include <memory>

namespace oregano {

/**
 * @brief Class for event handling base
 *
 * This class is responsible to create facilites to handle the event (pub/sub) system
 */
class IEventInterface {
public:
    virtual ~IEventInterface() = default;

    /**
     * Creates a publisher
     *
     * @return a unique_ptr to a new publisher
     */
    virtual std::unique_ptr<IPublisher> create_publisher() = 0;

    /**
     * Creates a subscriber
     *
     * @return a unique_ptr to a new subscriber
     */
    virtual std::unique_ptr<ISubscriber> create_subscriber() = 0;

    /**
     * Creates an instance of the interface
     *
     * @param p_message_broker_wrapper a wrapper over an actual message broker
     * @return a new instance
     */
    static std::unique_ptr<IEventInterface> create(IMessageBrokerWrapper& p_message_broker_wrapper);
};

} // namespace oregano