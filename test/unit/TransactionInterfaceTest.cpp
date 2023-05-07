#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../src/implementation/TransactionInterface.h"

#include "mocks/MockIEventInterface.h"
#include "mocks/MockIMessageBroker.h"
#include "mocks/MockIMessageBrokerWrapper.h"
#include "mocks/MockIMessageHandlerManager.h"
#include "mocks/MockIPublisher.h"
#include "mocks/MockISubscriber.h"

#include "utility.h"

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
using ::testing::Return;

class TransactionInterfaceTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        auto event_interface = oregano::test::make_strict_mock_unique_ptr(m_mock_event_interface);
        EXPECT_CALL(*m_mock_event_interface, create_publisher()).WillOnce([&m_mock_publisher = this->m_mock_publisher]() {
            return oregano::test::make_strict_mock_unique_ptr(m_mock_publisher);
        });
        auto subscriber = oregano::test::make_strict_mock_unique_ptr(m_mock_subscriber);
        EXPECT_CALL(*m_mock_event_interface, create_subscriber()).WillOnce([&subscriber]() { return std::move(subscriber); });

        EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker())
            .WillRepeatedly(
                [&m_mock_message_broker = this->m_mock_message_broker]() -> oregano::IMessageBroker& { return m_mock_message_broker; });

        EXPECT_CALL(m_mock_message_broker, register_transaction_callback(_)).WillOnce(Return("TRANSACTION_REGISTER_ID"));

        EXPECT_CALL(*m_mock_subscriber, set_on_message_callback(_)).Times(1);
        EXPECT_CALL(*m_mock_subscriber, subscribe(_)).Times(1);

        m_interface = std::make_unique<oregano::TransactionInterface>(m_mock_message_broker_wrapper,
            oregano::test::make_strict_mock_unique_ptr(m_mock_message_handler_manager), std::move(event_interface));
    }

    void TearDown() override { EXPECT_CALL(m_mock_message_broker, remove_transaction_callback("TRANSACTION_REGISTER_ID")).Times(1); }

    ::testing::StrictMock<oregano::MockIMessageBrokerWrapper> m_mock_message_broker_wrapper;
    ::testing::StrictMock<oregano::MockIMessageHandlerManager>* m_mock_message_handler_manager;
    ::testing::StrictMock<oregano::MockIPublisher>* m_mock_publisher;
    ::testing::StrictMock<oregano::MockISubscriber>* m_mock_subscriber;
    ::testing::StrictMock<oregano::MockIMessageBroker> m_mock_message_broker;
    ::testing::StrictMock<oregano::MockIEventInterface>* m_mock_event_interface;

    std::unique_ptr<oregano::TransactionInterface> m_interface;
};

TEST_F(TransactionInterfaceTest, GIVEN_TransactionInterfaceTest_WHEN_CreatingRequestSender_THEN_RequestSenderCreated)
{
    EXPECT_NE(m_interface->create_request_sender(), nullptr);
}

TEST_F(TransactionInterfaceTest, GIVEN_TransactionInterfaceTest_WHEN_CreatingRequestHandler_THEN_RequestHandlerCreated)
{
    EXPECT_NE(m_interface->create_request_handler(), nullptr);
}

TEST_F(TransactionInterfaceTest, GIVEN_Factory_WHEN_CreatingTransactionInterface_THEN_Success)
{
    EXPECT_CALL(m_mock_message_broker, register_transaction_callback(_)).WillOnce(Return("TRANSACTION_REGISTER_ID2"));
    EXPECT_CALL(m_mock_message_broker, remove_transaction_callback("TRANSACTION_REGISTER_ID2")).Times(1);
    EXPECT_CALL(m_mock_message_broker, register_event_callback(_)).WillOnce(Return("EVENT_REGISTER_ID"));
    EXPECT_CALL(m_mock_message_broker, remove_event_callback("EVENT_REGISTER_ID")).Times(1);
    EXPECT_CALL(m_mock_message_broker, subscribe(_)).Times(1);
    EXPECT_CALL(m_mock_message_broker, unsubscribe(_)).Times(1);
    EXPECT_NE(oregano::ITransactionInterface::create(m_mock_message_broker_wrapper), nullptr);
}
