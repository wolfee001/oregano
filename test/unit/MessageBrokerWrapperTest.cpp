#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../src/implementation/MessageBrokerWrapper.h"
#include "mocks/MockIMessageBroker.h"

#include "utility.h"

TEST(MessageBrokerWrapperTest, GIVEN_MessageBrokerWrapper_WHEN_GetMessageBroker_THEN_MessageBrokerReturned)
{
    ::testing::StrictMock<oregano::MockIMessageBroker>* broker;
    oregano::MessageBrokerWrapper wrapper(oregano::test::make_strict_mock_unique_ptr(broker));
    EXPECT_EQ(&wrapper.get_message_broker(), broker);
}
