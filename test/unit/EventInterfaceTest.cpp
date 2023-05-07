#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../src/implementation/EventInterface.h"

#include "mocks/MockIMessageBroker.h"
#include "mocks/MockIMessageBrokerWrapper.h"
#include "mocks/MockIMessageHandlerManager.h"

#include "utility.h"

using namespace std::string_literals;

using ::testing::_;
using ::testing::Eq;
using ::testing::Return;

class EventInterfaceTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        EXPECT_CALL(m_mock_broker, register_event_callback(_))
            .WillOnce(
                [&m_on_message_callback = this->m_on_message_callback](oregano::IMessageBroker::on_message_callback p_on_message_callback) {
                    m_on_message_callback = p_on_message_callback;
                    return "SOME_UNIQUE_ID"s;
                });
        EXPECT_CALL(m_mock_broker_wrapper, get_message_broker())
            .Times(1)
            .WillRepeatedly([&m_mock_broker = this->m_mock_broker]() -> oregano::IMessageBroker& { return m_mock_broker; });
        m_interface = std::make_unique<oregano::EventInterface>(
            m_mock_broker_wrapper, oregano::test::make_strict_mock_unique_ptr(m_mock_message_handler_manager));
    }

    void TearDown() override
    {
        EXPECT_CALL(m_mock_broker, remove_event_callback(Eq("SOME_UNIQUE_ID"s))).Times(1);
        EXPECT_CALL(m_mock_broker_wrapper, get_message_broker())
            .Times(1)
            .WillRepeatedly([&m_mock_broker = this->m_mock_broker]() -> oregano::IMessageBroker& { return m_mock_broker; });
        m_interface.reset();
    }

    std::unique_ptr<oregano::EventInterface> m_interface;
    ::testing::StrictMock<oregano::MockIMessageBrokerWrapper> m_mock_broker_wrapper;
    ::testing::StrictMock<oregano::MockIMessageBroker> m_mock_broker;
    ::testing::StrictMock<oregano::MockIMessageHandlerManager>* m_mock_message_handler_manager;
    oregano::IMessageBroker::on_message_callback m_on_message_callback;
};

TEST_F(EventInterfaceTest, GIVEN_Nothing_WHEN_EventInterfaceCreated_THEN_Success) { SUCCEED(); }

TEST_F(EventInterfaceTest, GIVEN_EventInterface_WHEN_MessageHappens_THEN_ForwardedToManager)
{
    EXPECT_CALL(*m_mock_message_handler_manager, on_message("channel"s, _)).Times(1);
    m_on_message_callback("channel"s, "message"s);
}

TEST_F(EventInterfaceTest, GIVEN_EventInterface_WHEN_CreatingPublisher_THEN_Success)
{
    EXPECT_CALL(m_mock_broker_wrapper, get_message_broker())
        .Times(1)
        .WillRepeatedly([&m_mock_broker = this->m_mock_broker]() -> oregano::IMessageBroker& { return m_mock_broker; });

    auto publisher = m_interface->create_publisher();
    EXPECT_NE(publisher, nullptr);
    EXPECT_CALL(m_mock_broker, publish(Eq("channel"s), _)).Times(1);
    publisher->publish("channel"s, "message"s);
}

TEST_F(EventInterfaceTest, GIVEN_EventInterface_WHEN_CreatingSubscriber_THEN_Success)
{
    auto subscriber = m_interface->create_subscriber();
    EXPECT_NE(subscriber, nullptr);
    EXPECT_CALL(*m_mock_message_handler_manager, add_message_handler(Eq("channel"s), _)).Times(1);
    EXPECT_CALL(*m_mock_message_handler_manager, remove_message_handler(_)).Times(1);
    subscriber->subscribe("channel"s);
}

TEST_F(EventInterfaceTest, GIVEN_Factory_WHEN_CreatingEventInterface_THEN_Success)
{
    EXPECT_CALL(m_mock_broker, register_event_callback(_)).WillOnce(Return("SOME_UNIQUE_ID"s));
    EXPECT_CALL(m_mock_broker, remove_event_callback(Eq("SOME_UNIQUE_ID"s))).Times(1);
    EXPECT_CALL(m_mock_broker_wrapper, get_message_broker())
        .Times(2)
        .WillRepeatedly([&m_mock_broker = this->m_mock_broker]() -> oregano::IMessageBroker& { return m_mock_broker; });

    EXPECT_NE(oregano::EventInterface::create(m_mock_broker_wrapper), nullptr);
}