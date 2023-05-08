#pragma once

#include "IMessageHandler.h"

#include <functional>
#include <string>

namespace oregano {

/**
 * @brief Class to subscribe to messages
 *
 * This is the interface of the subscriber side of the event system
 */
class ISubscriber : public IMessageHandler {
public:
    /**
     * @brief Request callback
     *
     * This type of function is called to handle an incoming message
     *
     * @param p_channel the channel the message arrived on
     * @param p_message the message itself
     */
    using on_message_callback = std::function<void(const std::string& p_channel, const std::string& p_message)>;

public:
    virtual ~ISubscriber() = default;

    /**
     * Subscribe to a channel
     *
     * @param p_channel the channel to subscribe to
     */
    virtual void subscribe(const std::string& p_channel) = 0;

    /**
     * Unsubscribe from a channel
     *
     * @param p_channel the channel to unsubscribe from
     */
    virtual void unsubscribe(const std::string& p_channel) = 0;

    /**
     * Sets the callback function for message handling
     *
     * @param p_callback the callback function to call on each message
     */
    virtual void set_on_message_callback(const on_message_callback& p_callback) = 0;
};

} // namespace oregano
