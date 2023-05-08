#include "Publisher.h"

#ifdef _MSC_VER
#pragma warning(disable : 4702)
#endif
#include <cppcodec/base64_rfc4648.hpp>
#ifdef _MSC_VER
#pragma warning(default : 4702)
#endif

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