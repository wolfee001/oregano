#include "EventInterface.h"

#include "IMessageBroker.h"
#include "MessageHandlerManager.h"
#include "Publisher.h"
#include "Subscriber.h"

namespace oregano {

EventInterface::EventInterface(IMessageBrokerWrapper& p_message_broker, std::unique_ptr<IMessageHandlerManager> p_message_handler_manager)
    : m_message_broker(p_message_broker)
    , m_message_handler_manager(std::move(p_message_handler_manager))
{
    m_callback_register_id = m_message_broker.get_message_broker().register_event_callback(
        [&m_message_handler_manager = this->m_message_handler_manager](
            const std::string& p_channel, const std::string& p_message) { m_message_handler_manager->on_message(p_channel, p_message); });
}

EventInterface::~EventInterface() { m_message_broker.get_message_broker().remove_event_callback(m_callback_register_id); }

std::unique_ptr<IPublisher> EventInterface::create_publisher()
{
    return std::make_unique<Publisher>(m_message_broker.get_message_broker());
}

std::unique_ptr<ISubscriber> EventInterface::create_subscriber() { return std::make_unique<Subscriber>(*m_message_handler_manager); }

std::unique_ptr<IEventInterface> IEventInterface::create(IMessageBrokerWrapper& p_message_broker)
{
    auto ret_val = std::make_unique<EventInterface>(
        p_message_broker, std::make_unique<MessageHandlerManager>(MessageHandlerManager::Type::Event, p_message_broker));
    return ret_val;
}

} // namespace oregano
