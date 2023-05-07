#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../src/implementation/RequestHandler.h"

#include "mocks/MockIMessageHandlerManager.h"
#include "mocks/MockIPublisher.h"

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

class RequestHandlerTest : public ::testing::Test {
protected:
    ::testing::StrictMock<oregano::MockIMessageHandlerManager> m_mock_message_handler_manager;
    ::testing::StrictMock<oregano::MockIPublisher> m_mock_publisher;
    oregano::RequestHandler m_handler { m_mock_message_handler_manager, m_mock_publisher };
};

TEST_F(RequestHandlerTest, GIVEN_RequestHander_WHEN_Listening_THEN_BrokerNotified)
{
    EXPECT_CALL(m_mock_message_handler_manager, add_message_handler(Eq("channel"s), _)).Times(1);
    m_handler.listen("channel");
}

TEST_F(RequestHandlerTest, GIVEN_RequestHander_WHEN_StopListening_THEN_BrokerNotified)
{
    EXPECT_CALL(m_mock_message_handler_manager, remove_message_handler(Eq("channel"s), _)).Times(1);
    m_handler.stop_listening("channel");
}

TEST_F(RequestHandlerTest, GIVEN_RequestHander_WHEN_AddMessageHandler_THEN_NoThrow)
{
    EXPECT_NO_THROW(m_handler.set_on_request_callback([](const std::string&, const std::string&) -> std::string { return {}; }));
}

TEST_F(RequestHandlerTest, GIVEN_RequestHander_WHEN_MessageArrives_THEN_CallbackCalled)
{
    m_handler.set_on_request_callback([](const std::string& p_channel, const std::string& p_message) -> std::string {
        EXPECT_EQ(p_channel, "RequestChannel"s);
        EXPECT_EQ(p_message, "SomePayload"s);

        return "SomeResponse"s;
    });

    auto response = nlohmann::json();
    response["header"]["id"] = "SomeId"s;
    response["payload"] = cppcodec::base64_rfc4648::encode("SomeResponse"s);
    EXPECT_CALL(m_mock_publisher, publish(Eq("SomeChannel"s), Eq(response.dump()))).Times(1);

    auto message = nlohmann::json();
    message["header"]["id"] = "SomeId"s;
    message["header"]["response_channel"] = "SomeChannel"s;
    message["payload"] = cppcodec::base64_rfc4648::encode("SomePayload"s);
    m_handler.on_message("RequestChannel"s, message.dump());
}

TEST_F(RequestHandlerTest, GIVEN_RequestHander_WHEN_MessageArrivesAndIsValidTimeout_THEN_CallbackCalled)
{
    m_handler.set_on_request_callback([](const std::string& p_channel, const std::string& p_message) -> std::string {
        EXPECT_EQ(p_channel, "RequestChannel"s);
        EXPECT_EQ(p_message, "SomePayload"s);

        return "SomeResponse"s;
    });

    auto response = nlohmann::json();
    response["header"]["id"] = "SomeId"s;
    response["payload"] = cppcodec::base64_rfc4648::encode("SomeResponse"s);
    EXPECT_CALL(m_mock_publisher, publish(Eq("SomeChannel"s), Eq(response.dump()))).Times(1);

    auto message = nlohmann::json();
    message["header"]["id"] = "SomeId"s;
    message["header"]["response_channel"] = "SomeChannel"s;
    message["header"]["valid_until"]
        = std::chrono::duration_cast<std::chrono::milliseconds>((std::chrono::system_clock::now() + 10s).time_since_epoch()).count();
    message["payload"] = cppcodec::base64_rfc4648::encode("SomePayload"s);
    m_handler.on_message("RequestChannel"s, message.dump());
}

TEST_F(RequestHandlerTest, GIVEN_RequestHander_WHEN_MessageArrivesAndIsNotValidTimeout_THEN_CallbackCalled)
{
    bool callback_called { false };
    m_handler.set_on_request_callback([&callback_called](const std::string&, const std::string&) -> std::string {
        callback_called = true;
        return "SomeResponse"s;
    });

    auto message = nlohmann::json();
    message["header"]["id"] = "SomeId"s;
    message["header"]["response_channel"] = "SomeChannel"s;
    message["header"]["valid_until"]
        = std::chrono::duration_cast<std::chrono::milliseconds>((std::chrono::system_clock::now() - 10s).time_since_epoch()).count();
    message["payload"] = cppcodec::base64_rfc4648::encode("SomePayload"s);
    m_handler.on_message("RequestChannel"s, message.dump());
    EXPECT_FALSE(callback_called);
}

TEST_F(RequestHandlerTest, GIVEN_RequestHander_WHEN_MessageArrivesAndNoCallbackSet_THEN_DefaultCallbackCalled)
{
    auto response = nlohmann::json();
    response["header"]["id"] = "SomeId"s;
    response["payload"] = cppcodec::base64_rfc4648::encode(""s);
    EXPECT_CALL(m_mock_publisher, publish(Eq("SomeChannel"s), Eq(response.dump()))).Times(1);

    auto message = nlohmann::json();
    message["header"]["id"] = "SomeId"s;
    message["header"]["response_channel"] = "SomeChannel"s;
    message["payload"] = cppcodec::base64_rfc4648::encode("SomePayload"s);
    m_handler.on_message("RequestChannel"s, message.dump());
}
