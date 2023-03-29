#pragma once

#include "IMessageBrokerWrapper.h"
#include "IRequestHandler.h"
#include "IRequestSender.h"

#include <memory>

namespace oregano {

class ITransactionInterface {
public:
    virtual ~ITransactionInterface() = default;

    virtual std::unique_ptr<IRequestSender> create_request_sender() = 0;
    virtual std::unique_ptr<IRequestHandler> create_request_handler() = 0;

    static std::unique_ptr<ITransactionInterface> create(IMessageBrokerWrapper& p_message_broker);
};

} // namespace oregano
