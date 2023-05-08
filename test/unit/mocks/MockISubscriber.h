#pragma once

#include <oregano/ISubscriber.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace oregano {

class MockISubscriber : public ISubscriber {
public:
    MOCK_METHOD(void, subscribe, (const std::string& p_channel), (override));
    MOCK_METHOD(void, unsubscribe, (const std::string& p_channel), (override));
    MOCK_METHOD(void, set_on_message_callback, (const on_message_callback& p_callback), (override));
    MOCK_METHOD(void, on_message, (const std::string& p_channel, const std::string& p_message), (override));
};

} // namespace oregano
