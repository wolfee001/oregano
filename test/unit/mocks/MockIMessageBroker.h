#pragma once

#include "../../../src/implementation/IMessageBroker.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace oregano {

class MockIMessageBroker : public IMessageBroker {
public:
    MOCK_METHOD(void, publish, (const std::string& p_channel, const std::string& p_message), (override));
    MOCK_METHOD(void, subscribe, (const std::string& p_channel), (override));
    MOCK_METHOD(void, unsubscribe, (const std::string& p_channel), (override));
    MOCK_METHOD(std::string, register_event_callback, (const on_message_callback& p_callback), (override));
    MOCK_METHOD(void, remove_event_callback, (const std::string& p_id), (override));

    MOCK_METHOD(void, send_request, (const std::string& p_queue, const std::string& p_message), (override));
    MOCK_METHOD(void, listen, (const std::string& p_queue), (override));
    MOCK_METHOD(void, stop_listening, (const std::string& p_queue), (override));
    MOCK_METHOD(std::string, register_transaction_callback, (const on_message_callback& p_callback), (override));
    MOCK_METHOD(void, remove_transaction_callback, (const std::string& p_id), (override));
};

} // namespace oregano
