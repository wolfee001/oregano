#pragma once

#include <oregano/IEventInterface.h>
#include <oregano/ITransactionInterface.h>

#include "IMessageHandlerManager.h"

#include <functional>
#include <mutex>
#include <unordered_map>

namespace oregano {

class TransactionInterface : public ITransactionInterface {
public:
    explicit TransactionInterface(IMessageBrokerWrapper& p_message_broker,
        std::unique_ptr<IMessageHandlerManager> p_message_handler_manager, std::unique_ptr<IEventInterface> p_event_interface);
    ~TransactionInterface() override;

    std::unique_ptr<IRequestSender> create_request_sender() override;
    std::unique_ptr<IRequestHandler> create_request_handler() override;

private:
    IMessageBrokerWrapper& m_message_broker;
    std::unique_ptr<IMessageHandlerManager> m_message_handler_manager;
    std::unique_ptr<IEventInterface> m_event_interface;
    std::string m_callback_register_id;
    std::string m_response_channel;
    std::unique_ptr<IPublisher> m_publisher;
    std::unique_ptr<ISubscriber> m_subscriber;
    std::unordered_map<std::string, IResponsePromise*> m_on_response_callbacks;
    std::mutex m_on_response_callbacks_lock;
};

} // namespace oregano
