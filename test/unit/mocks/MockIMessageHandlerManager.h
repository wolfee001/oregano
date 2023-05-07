#pragma once

#include "../../../src/implementation/IMessageHandlerManager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace oregano {

class MockIMessageHandlerManager : public IMessageHandlerManager {
public:
    MOCK_METHOD(
        void, add_message_handler, (const std::string& p_channel, std::reference_wrapper<IMessageHandler> p_subscriber), (override));
    MOCK_METHOD(
        void, remove_message_handler, (const std::string& p_channel, std::reference_wrapper<IMessageHandler> p_subscriber), (override));
    MOCK_METHOD(void, remove_message_handler, (std::reference_wrapper<IMessageHandler> p_subscriber), (override));
    MOCK_METHOD(void, on_message, (const std::string& p_channel, const std::string& p_message), (override));
};

} // namespace oregano
