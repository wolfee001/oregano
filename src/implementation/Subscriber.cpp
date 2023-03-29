#include "Subscriber.h"

#include "IMessageBroker.h"

#include <cppcodec/base64_rfc4648.hpp>

namespace oregano {

Subscriber::Subscriber(IMessageHandlerManager& p_message_handler_manager)
    : m_message_handler_manager(p_message_handler_manager)
{
}

Subscriber::~Subscriber() { m_message_handler_manager.remove_message_handler(*this); }

void Subscriber::subscribe(const std::string& p_channel) { m_message_handler_manager.add_message_handler(p_channel, *this); }

void Subscriber::unsubscribe(const std::string& p_channel) { m_message_handler_manager.remove_message_handler(p_channel, *this); }

void Subscriber::set_on_message_callback(const on_message_callback& p_callback) { m_on_message_callback = p_callback; }

void Subscriber::on_message(const std::string& p_channel, const std::string& p_message)
{
    m_on_message_callback(p_channel, cppcodec::base64_rfc4648::decode<std::string>(p_message));
}

} // namespace oregano