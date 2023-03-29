#pragma once

#include <oregano/IEventInterface.h>

#include "IMessageHandlerManager.h"

namespace oregano {

class EventInterface : public IEventInterface {
public:
    explicit EventInterface(IMessageBrokerWrapper& p_message_broker, std::unique_ptr<IMessageHandlerManager> p_message_handler_manager);
    ~EventInterface() override;

    std::unique_ptr<IPublisher> create_publisher() override;
    std::unique_ptr<ISubscriber> create_subscriber() override;

private:
    IMessageBrokerWrapper& m_message_broker;
    std::unique_ptr<IMessageHandlerManager> m_message_handler_manager;
    std::string m_callback_register_id;
};

} // namespace oregano
