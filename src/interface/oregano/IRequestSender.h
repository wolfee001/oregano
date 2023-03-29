#pragma once

#include "IResponsePromise.h"

namespace oregano {

class IRequestSender {
public:
    virtual ~IRequestSender() = default;

    virtual std::unique_ptr<IResponsePromise> send_request(
        const std::string& p_channel, const std::string& p_message, std::chrono::milliseconds p_timeout)
        = 0;
};

} // namespace oregano