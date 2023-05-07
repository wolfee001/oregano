#pragma once

#include <oregano/IMessageHandler.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace oregano {

class MockIMessageHandler : public IMessageHandler {
public:
    MOCK_METHOD(void, on_message, (const std::string& p_channel, const std::string& p_message), (override));
};

} // namespace oregano
