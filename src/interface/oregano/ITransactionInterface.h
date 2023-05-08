#pragma once

#include "IMessageBrokerWrapper.h"
#include "IRequestHandler.h"
#include "IRequestSender.h"

#include <memory>

namespace oregano {

/**
 * @brief Class for transaction handling base
 *
 * This class is responsible to create facilites to handle the transaction (request/response) system
 */
class ITransactionInterface {
public:
    virtual ~ITransactionInterface() = default;

    /**
     * Creates a request sender
     *
     * @return a unique_ptr to a new request sender
     */
    virtual std::unique_ptr<IRequestSender> create_request_sender() = 0;

    /**
     * Creates a request handler
     *
     * @return a unique_ptr to a new request handler
     */
    virtual std::unique_ptr<IRequestHandler> create_request_handler() = 0;

    /**
     * Creates an instance of the interface
     *
     * @param p_message_broker_wrapper a wrapper over an actual message broker
     * @return a new instance
     */
    static std::unique_ptr<ITransactionInterface> create(IMessageBrokerWrapper& p_message_broker_wrapper);
};

} // namespace oregano
