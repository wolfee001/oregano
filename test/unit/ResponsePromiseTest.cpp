#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../src/implementation/ResponsePromise.h"

#include "mocks/MockIMessageBroker.h"

#ifdef _MSC_VER
#pragma warning(disable : 4702)
#endif
#include <cppcodec/base64_rfc4648.hpp>
#ifdef _MSC_VER
#pragma warning(default : 4702)
#endif

#include <nlohmann/json.hpp>

#include <mutex>

using namespace std::string_literals;
using namespace std::chrono_literals;

using ::testing::_;
using ::testing::Eq;

class ResponsePromiseTest : public ::testing::Test {
};

TEST_F(ResponsePromiseTest, GIVEN_ResponsePromise_WHEN_Deleted_THEN_CallbackCalled)
{
    auto called { false };
    const auto unreigster_callback = [&called]() { called = true; };
    auto promise = std::make_unique<oregano::ResponsePromise>(unreigster_callback, std::chrono::system_clock::now());
    promise.reset();
    EXPECT_TRUE(called);
}

TEST_F(ResponsePromiseTest, GIVEN_ResponsePromise_WHEN_TimedOutAndNoResponse_THEN_ReturnTimeout)
{
    auto promise = oregano::ResponsePromise([]() {}, std::chrono::system_clock::now() - 10s);
    auto start = std::chrono::system_clock::now();
    EXPECT_EQ(promise.await().resolution, oregano::IResponsePromise::Resolution::Timeout);
    EXPECT_LT(std::chrono::system_clock::now() - start, 10ms);
}

TEST_F(ResponsePromiseTest, GIVEN_ResponsePromise_WHEN_TimedOutAndResponse_THEN_ReturnTimeout)
{
    auto promise = oregano::ResponsePromise([]() {}, std::chrono::system_clock::now() - 10s);
    promise.on_response("SomeResponse");
    auto start = std::chrono::system_clock::now();
    EXPECT_EQ(promise.await().resolution, oregano::IResponsePromise::Resolution::Timeout);
    EXPECT_LT(std::chrono::system_clock::now() - start, 10ms);
}

TEST_F(ResponsePromiseTest, GIVEN_ResponsePromise_WHEN_NoResponse_THEN_WaitForTimeout)
{
    auto promise = oregano::ResponsePromise([]() {}, std::chrono::system_clock::now() + 500ms);
    auto start = std::chrono::system_clock::now();
    EXPECT_EQ(promise.await().resolution, oregano::IResponsePromise::Resolution::Timeout);
    EXPECT_GE(std::chrono::system_clock::now() - start, 450ms);
}

TEST_F(ResponsePromiseTest, GIVEN_ResponsePromise_WHEN_NoResponse_THEN_WaitForTimeoutWithThread)
{
    auto called { false };
    auto promise = std::make_unique<oregano::ResponsePromise>([]() {}, std::chrono::system_clock::now() + 500ms);
    promise->then([&called](oregano::IResponsePromise::response_t p_response) {
        called = true;
        EXPECT_EQ(p_response.resolution, oregano::IResponsePromise::Resolution::Timeout);
        EXPECT_EQ(p_response.message, std::nullopt);
    });
    auto start = std::chrono::system_clock::now();
    EXPECT_NO_THROW(promise.reset());
    EXPECT_GE(std::chrono::system_clock::now() - start, 450ms);
    EXPECT_TRUE(called);
}

TEST_F(ResponsePromiseTest, GIVEN_ResponsePromise_WHEN_Response_THEN_ReturnResponse)
{
    auto promise = oregano::ResponsePromise([]() {}, std::chrono::system_clock::now() + 10s);
    promise.on_response("SomeResponse");
    auto start = std::chrono::system_clock::now();
    auto response = promise.await();
    EXPECT_LT(std::chrono::system_clock::now() - start, 10ms);
    EXPECT_EQ(response.resolution, oregano::IResponsePromise::Resolution::Answer);
    EXPECT_EQ(response.message, "SomeResponse"s);
}

TEST_F(ResponsePromiseTest, GIVEN_ResponsePromise_WHEN_Response_THEN_ReturnResponseWithThread)
{
    std::timed_mutex mtx;
    mtx.lock();
    auto start = std::chrono::system_clock::now();
    auto promise = oregano::ResponsePromise([]() {}, std::chrono::system_clock::now() + 10s);
    promise.then([&start, &mtx](oregano::IResponsePromise::response_t p_response) {
        EXPECT_EQ(p_response.resolution, oregano::IResponsePromise::Resolution::Answer);
        EXPECT_EQ(p_response.message, "SomeResponse"s);
        EXPECT_LT(std::chrono::system_clock::now() - start, 50ms);
        mtx.unlock();
    });
    promise.on_response("SomeResponse");
    if (!mtx.try_lock_for(10s)) {
        FAIL();
    }
}
