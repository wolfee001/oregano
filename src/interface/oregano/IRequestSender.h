#pragma once

#include "IResponsePromise.h"

#include <memory>

namespace oregano {

/**
 * @brief Class to send requests
 *
 * This is the interface of the producer side of the transaction system
 */
class IRequestSender {
public:
    virtual ~IRequestSender() = default;

    /**
     * Sends a request
     *
     * @param p_channel the target channel
     * @param p_message the request message
     * @param p_timeout the timeout of the promise
     * @return a promise to the response
     */
    virtual std::unique_ptr<IResponsePromise> send_request(
        const std::string& p_channel, const std::string& p_message, std::chrono::milliseconds p_timeout)
        = 0;
};

} // namespace oregano