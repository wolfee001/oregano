#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../src/implementation/Subscriber.h"

#include "mocks/MockIMessageHandlerManager.h"

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

class SubscriberTest : public ::testing::Test {
protected:
    void SetUp() override { EXPECT_CALL(m_mock_message_handler, remove_message_handler(_)).Times(1); }

    ::testing::StrictMock<oregano::MockIMessageHandlerManager> m_mock_message_handler;
    oregano::Subscriber m_subscriber { m_mock_message_handler };
};

TEST_F(SubscriberTest, GIVEN_Subscriber_WHEN_Subscribe_THEN_CallToHandler)
{
    EXPECT_CALL(m_mock_message_handler, add_message_handler(Eq("channel"s), _)).Times(1);
    m_subscriber.subscribe("channel");
}

TEST_F(SubscriberTest, GIVEN_Subscriber_WHEN_Unsubscribe_THEN_CallToHandler)
{
    EXPECT_CALL(m_mock_message_handler, remove_message_handler(Eq("channel"s), _)).Times(1);
    m_subscriber.unsubscribe("channel");
}

TEST_F(SubscriberTest, GIVEN_Subscriber_WHEN_SetMessageCallback_THEN_NoExcept)
{
    EXPECT_NO_THROW(m_subscriber.set_on_message_callback([](const std::string&, const std::string&) {}));
}

TEST_F(SubscriberTest, GIVEN_Subscriber_WHEN_OnMessgae_THEN_CallbackCalled)
{
    auto called { false };
    m_subscriber.set_on_message_callback([&called](const std::string& p_channel, const std::string& p_message) {
        called = true;
        EXPECT_EQ(p_channel, "channel"s);
        EXPECT_EQ(p_message, "message"s);
    });

    m_subscriber.on_message("channel", cppcodec::base64_rfc4648::encode("message"s));
    EXPECT_TRUE(called);
}
