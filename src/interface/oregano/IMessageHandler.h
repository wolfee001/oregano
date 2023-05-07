#pragma once

#include <string>

namespace oregano {

/**
 * @brief Abstract class to handle messages
 *
 * This class should not be used directly
 */
class IMessageHandler {
public:
    virtual ~IMessageHandler() = default;

    /**
     * Callback function on message arrival
     *
     * @param p_channel the channel of the message
     * @param p_message the message itself
     */
    virtual void on_message(const std::string& p_channel, const std::string& p_message) = 0;
};

} // namespace oregano
