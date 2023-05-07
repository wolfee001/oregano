#pragma once

#include <oregano/IMessageBrokerWrapper.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace oregano {

class MockIMessageBrokerWrapper : public IMessageBrokerWrapper {
public:
    MOCK_METHOD(IMessageBroker&, get_message_broker, (), (override));
};

} // namespace oregano
