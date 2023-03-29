#include "Publisher.h"

#include <cppcodec/base64_rfc4648.hpp>

#include <memory>

namespace oregano {

Publisher::Publisher(IMessageBroker& p_message_broker)
    : m_message_broker(p_message_broker)
{
}

void Publisher::publish(const std::string& p_channel, const std::string& p_message)
{
    m_message_broker.publish(p_channel, cppcodec::base64_rfc4648::encode(p_message));
}

} // namespace oregano