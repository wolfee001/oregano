#include "MessageBrokerWrapper.h"

namespace oregano {

MessageBrokerWrapper::MessageBrokerWrapper(std::unique_ptr<IMessageBroker> p_message_broker)
    : m_message_broker(std::move(p_message_broker))
{
}

IMessageBroker& MessageBrokerWrapper::get_message_broker() { return *m_message_broker; }

std::unique_ptr<IMessageBrokerWrapper> IMessageBrokerWrapper::create(broker_configuration::Configuration p_configuration)
{
    auto broker = IMessageBroker::create(p_configuration);
    broker->init();
    return std::make_unique<MessageBrokerWrapper>(std::move(broker));
}

} // namespace oregano