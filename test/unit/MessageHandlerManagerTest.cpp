#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../src/implementation/MessageHandlerManager.h"

#include "mocks/MockIMessageBroker.h"
#include "mocks/MockIMessageBrokerWrapper.h"
#include "mocks/MockIMessageHandler.h"

#include "utility.h"

using namespace std::string_literals;

using ::testing::Eq;
using ::testing::Return;

class MessageHandlerManagerTest : public ::testing::Test {
protected:
    ::testing::StrictMock<oregano::MockIMessageBroker> m_mock_message_broker;
    ::testing::StrictMock<oregano::MockIMessageBrokerWrapper> m_mock_message_broker_wrapper;
    oregano::MessageHandlerManager m_event_manager { oregano::MessageHandlerManager::Type::Event, m_mock_message_broker_wrapper };
    oregano::MessageHandlerManager m_transaction_manager { oregano::MessageHandlerManager::Type::Transaction,
        m_mock_message_broker_wrapper };
    oregano::MessageHandlerManager m_unknown_manager { oregano::MessageHandlerManager::Type { 42 }, m_mock_message_broker_wrapper };
};

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_AddFirstHandler_THEN_Subscribe)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).WillOnce([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel"s))).Times(1);
    m_event_manager.add_message_handler("channel", subscriber);
}

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_AddTwoHandlers_THEN_OnlyOneSubscribe)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber1;
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber2;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).WillOnce([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel"s))).Times(1);
    m_event_manager.add_message_handler("channel", subscriber1);
    m_event_manager.add_message_handler("channel", subscriber2);
}

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_AddTwoHandlersDifferentChannel_THEN_TwoSubscribe)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber1;
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber2;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).Times(2).WillRepeatedly([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel1"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel2"s))).Times(1);
    m_event_manager.add_message_handler("channel1", subscriber1);
    m_event_manager.add_message_handler("channel2", subscriber2);
}

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_AddTwoHandlersDifferentChannel2_THEN_TwoSubscribe)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber1;
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber2;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).Times(2).WillRepeatedly([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel1"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel2"s))).Times(1);
    m_event_manager.add_message_handler("channel1", subscriber1);
    m_event_manager.add_message_handler("channel2", subscriber1);
    m_event_manager.add_message_handler("channel2", subscriber2);
}

TEST_F(MessageHandlerManagerTest, GIVEN_TransactionManager_WHEN_AddFirstHandler_THEN_Listen)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).WillOnce([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, listen(Eq("channel"s))).Times(1);
    m_transaction_manager.add_message_handler("channel", subscriber);
}

TEST_F(MessageHandlerManagerTest, GIVEN_TransactionManager_WHEN_AddTwoHandlers_THEN_Exception)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber1;
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber2;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).WillOnce([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, listen(Eq("channel"s))).Times(1);
    m_transaction_manager.add_message_handler("channel", subscriber1);
    EXPECT_THROW(m_transaction_manager.add_message_handler("channel", subscriber2), oregano::MessageHandlerManagerException);
}

TEST_F(MessageHandlerManagerTest, GIVEN_TransactionManager_WHEN_AddTwoHandlersDifferentChannel_THEN_TwoListen)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber1;
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber2;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).Times(2).WillRepeatedly([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, listen(Eq("channel1"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, listen(Eq("channel2"s))).Times(1);
    m_transaction_manager.add_message_handler("channel1", subscriber1);
    m_transaction_manager.add_message_handler("channel2", subscriber2);
}

TEST_F(MessageHandlerManagerTest, GIVEN_TransactionManager_WHEN_AddTwoHandlersDifferentChannel2_THEN_Exception)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber1;
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber2;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).Times(2).WillRepeatedly([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, listen(Eq("channel1"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, listen(Eq("channel2"s))).Times(1);
    m_transaction_manager.add_message_handler("channel1", subscriber1);
    m_transaction_manager.add_message_handler("channel2", subscriber1);
    EXPECT_THROW(m_transaction_manager.add_message_handler("channel2", subscriber2), oregano::MessageHandlerManagerException);
}

TEST_F(MessageHandlerManagerTest, GIVEN_UnknownManager_WHEN_AddFirstHandler_THEN_Excetption)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber;
    EXPECT_THROW(m_unknown_manager.add_message_handler("channel", subscriber), oregano::MessageHandlerManagerException);
}

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_RemoveHandlerWithChannelWithNoSubscription_THEN_DontUnsubscribe)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber1;
    m_event_manager.remove_message_handler("channel", subscriber1);
}

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_RemoveHandlerWithNoSubscription_THEN_DontUnsubscribe)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber1;
    m_event_manager.remove_message_handler(subscriber1);
}

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_RemoveHandlerWithChannelFromMultiple_THEN_DontUnsubscribe)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber1;
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber2;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).WillOnce([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel"s))).Times(1);
    m_event_manager.add_message_handler("channel", subscriber1);
    m_event_manager.add_message_handler("channel", subscriber2);
    m_event_manager.remove_message_handler("channel", subscriber1);
}

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_RemoveHandlerFromMultiple_THEN_DontUnsubscribe)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber1;
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber2;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).WillOnce([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel"s))).Times(1);
    m_event_manager.add_message_handler("channel", subscriber1);
    m_event_manager.add_message_handler("channel", subscriber2);
    m_event_manager.remove_message_handler(subscriber1);
}

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_RemoveHandlerWithChannel_THEN_Unsubscribe)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber1;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).Times(2).WillRepeatedly([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, unsubscribe(Eq("channel"s))).Times(1);
    m_event_manager.add_message_handler("channel", subscriber1);
    m_event_manager.remove_message_handler("channel", subscriber1);
}

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_RemoveHandler_THEN_Unsubscribe)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber1;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).Times(2).WillRepeatedly([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, unsubscribe(Eq("channel"s))).Times(1);
    m_event_manager.add_message_handler("channel", subscriber1);
    m_event_manager.remove_message_handler(subscriber1);
}

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_RemoveHandlerWithMultipleChannels_THEN_Unsubscribe)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber1;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).WillRepeatedly([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel1"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel2"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, unsubscribe(Eq("channel1"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, unsubscribe(Eq("channel2"s))).Times(1);
    m_event_manager.add_message_handler("channel1", subscriber1);
    m_event_manager.add_message_handler("channel2", subscriber1);
    m_event_manager.remove_message_handler(subscriber1);
}

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_RemoveHandlerWithMultipleChannels2_THEN_Unsubscribe)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber1;
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber2;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).WillRepeatedly([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel1"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel2"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, unsubscribe(Eq("channel1"s))).Times(1);
    m_event_manager.add_message_handler("channel1", subscriber1);
    m_event_manager.add_message_handler("channel2", subscriber1);
    m_event_manager.add_message_handler("channel2", subscriber2);
    m_event_manager.remove_message_handler(subscriber1);
}

TEST_F(MessageHandlerManagerTest, GIVEN_TransactionManager_WHEN_RemoveHandlerWithChannel_THEN_Unsubscribe)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber1;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).Times(2).WillRepeatedly([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, listen(Eq("channel"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, stop_listening(Eq("channel"s))).Times(1);
    m_transaction_manager.add_message_handler("channel", subscriber1);
    m_transaction_manager.remove_message_handler("channel", subscriber1);
}

TEST_F(MessageHandlerManagerTest, GIVEN_TransactionManager_WHEN_RemoveHandler_THEN_Unsubscribe)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber1;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).Times(2).WillRepeatedly([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, listen(Eq("channel"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, stop_listening(Eq("channel"s))).Times(1);
    m_transaction_manager.add_message_handler("channel", subscriber1);
    m_transaction_manager.remove_message_handler(subscriber1);
}

TEST_F(MessageHandlerManagerTest, GIVEN_TransactionManager_WHEN_RemoveHandlerWithMultipleChannels_THEN_Unsubscribe)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber1;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).WillRepeatedly([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, listen(Eq("channel1"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, listen(Eq("channel2"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, stop_listening(Eq("channel1"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, stop_listening(Eq("channel2"s))).Times(1);
    m_transaction_manager.add_message_handler("channel1", subscriber1);
    m_transaction_manager.add_message_handler("channel2", subscriber1);
    m_transaction_manager.remove_message_handler(subscriber1);
}

TEST_F(MessageHandlerManagerTest, GIVEN_UnknownManager_WHEN_RemoveHandler_THEN_Excetption)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber;
    EXPECT_THROW(m_unknown_manager.add_message_handler("channel", subscriber), oregano::MessageHandlerManagerException);
    EXPECT_THROW(m_unknown_manager.remove_message_handler("channel", subscriber), oregano::MessageHandlerManagerException);
    EXPECT_THROW(m_unknown_manager.add_message_handler("channel", subscriber), oregano::MessageHandlerManagerException);
    EXPECT_THROW(m_unknown_manager.remove_message_handler(subscriber), oregano::MessageHandlerManagerException);
}

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_MessageArrivesAndNoSubscribers_THEN_NothingHappens)
{
    EXPECT_NO_THROW(m_event_manager.on_message("channel", "message"));
}

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_MessageArrivesAndSingleSubscriber_THEN_Dispatch)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).WillOnce([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel"s))).Times(1);
    EXPECT_CALL(subscriber, on_message(Eq("channel"s), Eq("message"s))).Times(1);

    m_event_manager.add_message_handler("channel", subscriber);
    m_event_manager.on_message("channel", "message");
}

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_MessageArrivesAndSingleSubscriberWithMultipleSubscription_THEN_Dispatch)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).WillRepeatedly([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel"s))).Times(1);
    EXPECT_CALL(subscriber, on_message(Eq("channel"s), Eq("message"s))).Times(1);

    m_event_manager.add_message_handler("channel", subscriber);
    m_event_manager.add_message_handler("channel", subscriber);
    m_event_manager.on_message("channel", "message");
}

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_MessageArrivesMultipleTimesAndSingleSubscriber_THEN_Dispatch)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).WillOnce([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel"s))).Times(1);
    EXPECT_CALL(subscriber, on_message(Eq("channel"s), Eq("message"s))).Times(2);

    m_event_manager.add_message_handler("channel", subscriber);
    m_event_manager.on_message("channel", "message");
    m_event_manager.on_message("channel", "message");
}

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_MessageArrivesMultipleSubscriber_THEN_Dispatch)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber1;
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber2;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).WillRepeatedly([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel"s))).Times(1);
    EXPECT_CALL(subscriber1, on_message(Eq("channel"s), Eq("message"s))).Times(1);
    EXPECT_CALL(subscriber2, on_message(Eq("channel"s), Eq("message"s))).Times(1);

    m_event_manager.add_message_handler("channel", subscriber1);
    m_event_manager.add_message_handler("channel", subscriber2);
    m_event_manager.on_message("channel", "message");
}

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_MessageArrivesMultipleSubscriberDifferentChannels_THEN_Dispatch)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber1;
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber2;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).WillRepeatedly([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel1"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel2"s))).Times(1);
    EXPECT_CALL(subscriber1, on_message(Eq("channel1"s), Eq("message"s))).Times(1);

    m_event_manager.add_message_handler("channel1", subscriber1);
    m_event_manager.add_message_handler("channel2", subscriber2);
    m_event_manager.on_message("channel1", "message");
}

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_MessageArrivesAndSingleSubscriber_THEN_NoDispatchAfterUnsubscribe)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).WillRepeatedly([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel1"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel2"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, unsubscribe(Eq("channel1"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, unsubscribe(Eq("channel2"s))).Times(1);
    EXPECT_CALL(subscriber, on_message(Eq("channel1"s), Eq("message"s))).Times(1);
    EXPECT_CALL(subscriber, on_message(Eq("channel2"s), Eq("message"s))).Times(1);

    m_event_manager.add_message_handler("channel1", subscriber);
    m_event_manager.add_message_handler("channel2", subscriber);
    m_event_manager.on_message("channel1", "message");
    m_event_manager.on_message("channel2", "message");
    m_event_manager.remove_message_handler(subscriber);
    m_event_manager.on_message("channel1", "message");
    m_event_manager.on_message("channel2", "message");
}

TEST_F(MessageHandlerManagerTest, GIVEN_EventManager_WHEN_MessageArrivesAndSingleSubscriber_THEN_NoDispatchAfterUnsubscribeFromChannel)
{
    ::testing::StrictMock<oregano::MockIMessageHandler> subscriber;
    EXPECT_CALL(m_mock_message_broker_wrapper, get_message_broker()).WillRepeatedly([&]() -> oregano::IMessageBroker& {
        return m_mock_message_broker;
    });
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel1"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, subscribe(Eq("channel2"s))).Times(1);
    EXPECT_CALL(m_mock_message_broker, unsubscribe(Eq("channel1"s))).Times(1);
    EXPECT_CALL(subscriber, on_message(Eq("channel1"s), Eq("message"s))).Times(1);
    EXPECT_CALL(subscriber, on_message(Eq("channel2"s), Eq("message"s))).Times(2);

    m_event_manager.add_message_handler("channel1", subscriber);
    m_event_manager.add_message_handler("channel2", subscriber);
    m_event_manager.on_message("channel1", "message");
    m_event_manager.on_message("channel2", "message");
    m_event_manager.remove_message_handler("channel1", subscriber);
    m_event_manager.on_message("channel1", "message");
    m_event_manager.on_message("channel2", "message");
}
