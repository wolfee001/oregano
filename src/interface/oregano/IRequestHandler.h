#pragma once

#include "IMessageHandler.h"

#include <functional>
#include <string>

namespace oregano {

/**
 * @brief Class for request handling
 *
 * An actual implementation of this class is used as the facility for the request handler of the transaction interface
 */
class IRequestHandler : public IMessageHandler {
public:
    /**
     * @brief Request callback
     *
     * This type of function is called to process an incoming request
     *
     * @param p_channel the channel the message arrived on
     * @param p_message the request message itself
     * @return the message to send back
     */
    using on_request_callback = std::function<std::string(const std::string& p_channel, const std::string& p_message)>;

public:
    virtual ~IRequestHandler() = default;

    /**
     * Start listen to messages on the given channel
     *
     * @param p_channel the channel to listen on
     */
    virtual void listen(const std::string& p_channel) = 0;

    /**
     * Stops listening on the given channel
     *
     * @param p_channel the channel to stop listening on
     */
    virtual void stop_listening(const std::string& p_channel) = 0;

    /**
     * Sets the callback function for message handling
     *
     * @param p_callback the callback function to call on each message to process
     */
    virtual void set_on_request_callback(const on_request_callback& p_callback) = 0;
};

} // namespace oregano
