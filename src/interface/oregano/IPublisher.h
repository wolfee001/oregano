#pragma once

#include <string>

namespace oregano {

/**
 * @brief Class to publish messages
 *
 * This is the interface of the publisher side of the event system
 */
class IPublisher {
public:
    virtual ~IPublisher() = default;

    /**
     * Publish a message to a channel
     *
     * @param p_channel the channel to publish onto
     * @param p_message the message to publish
     */
    virtual void publish(const std::string& p_channel, const std::string& p_message) = 0;
};

} // namespace oregano