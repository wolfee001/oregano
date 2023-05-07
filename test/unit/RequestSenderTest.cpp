#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../src/implementation/RequestSender.h"

#include "mocks/MockIMessageBroker.h"

#ifdef _MSC_VER
#pragma warning(disable : 4702)
#endif
#include <cppcodec/base64_rfc4648.hpp>
#ifdef _MSC_VER
#pragma warning(default : 4702)
#endif

#include <nlohmann/json.hpp>

using namespace std::string_literals;
using namespace std::chrono_literals;

using ::testing::_;
using ::testing::Eq;

class RequestSenderTest : public ::testing::Test {
protected:
    ::testing::StrictMock<oregano::MockIMessageBroker> m_mock_message_broker;
    std::string m_response_channel { "ResponseChannel" };
    size_t m_register_call { 0 };
    size_t m_unregister_call { 0 };
    oregano::RequestSender::response_promise_register_callback m_register_callback
        = [&m_register_call = this->m_register_call](const std::string&, oregano::IResponsePromise*) { m_register_call++; };
    oregano::RequestSender::response_promise_unregister_callback m_unregister_callback
        = [&m_unregister_call = this->m_unregister_call](const std::string&) { m_unregister_call++; };
    oregano::RequestSender m_sender { m_mock_message_broker, m_response_channel, m_register_callback, m_unregister_callback };
};

TEST_F(RequestSenderTest, GIVEN_RequestSender_WHEN_SendRequest_THEN_RequestSent)
{
    EXPECT_CALL(m_mock_message_broker, send_request(Eq("channel"s), _)).WillOnce([](const std::string&, const std::string& p_message) {
        auto message = nlohmann::json().parse(p_message);
        EXPECT_EQ(message["payload"], cppcodec::base64_rfc4648::encode("message"s));
    });

    auto promise = m_sender.send_request("channel", "message", 5s);
    EXPECT_NE(promise, nullptr);
    EXPECT_EQ(m_register_call, 1);
    EXPECT_EQ(m_unregister_call, 0);
}
