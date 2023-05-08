#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../src/implementation/Publisher.h"

#include "mocks/MockIMessageBroker.h"

#ifdef _MSC_VER
#pragma warning(disable : 4702)
#endif
#include <cppcodec/base64_rfc4648.hpp>
#ifdef _MSC_VER
#pragma warning(default : 4702)
#endif

using namespace std::string_literals;

using ::testing::_;
using ::testing::Eq;

class PublisherTest : public ::testing::Test {
protected:
    ::testing::StrictMock<oregano::MockIMessageBroker> m_mock_message_broker;
    oregano::Publisher m_publisher { m_mock_message_broker };
};

TEST_F(PublisherTest, GIVEN_Publisher_WHEN_Publish_THEN_BrokerNotified)
{
    EXPECT_CALL(m_mock_message_broker, publish(Eq("channel"s), Eq(cppcodec::base64_rfc4648::encode("message"s)))).Times(1);
    m_publisher.publish("channel", "message");
}
