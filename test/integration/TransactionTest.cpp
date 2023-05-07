#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <oregano/BrokerConfiguration.h>
#include <oregano/IEventInterface.h>
#include <oregano/IMessageBrokerWrapper.h>
#include <oregano/ITransactionInterface.h>

#include <chrono>
#include <thread>

using namespace std::chrono_literals;
using namespace std::string_literals;

class TransactionTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        m_configuration = oregano::broker_configuration::Redis::create();
        m_configuration->set_host("127.0.0.1");
        m_configuration->set_port(6379);
        m_broker = oregano::IMessageBrokerWrapper::create(*m_configuration);
    }

    std::unique_ptr<oregano::broker_configuration::Redis> m_configuration;
    std::unique_ptr<oregano::IMessageBrokerWrapper> m_broker;
};

TEST_F(TransactionTest, Test)
{
    auto transaction_interface = oregano::ITransactionInterface::create(*m_broker);

    std::timed_mutex mutex;

    auto request_handler = transaction_interface->create_request_handler();
    request_handler->set_on_request_callback([&mutex](const std::string& p_channel, const std::string& p_message) {
        std::lock_guard<std::timed_mutex> g { mutex };
        EXPECT_EQ(p_channel, "command_channel"s);
        if (p_message == "timeout"s) {
            std::this_thread::sleep_for(1s);
        }
        return "some response";
    });
    request_handler->listen("command_channel");

    auto request_sender = transaction_interface->create_request_sender();

    {
        auto promise = request_sender->send_request("command_channel", "message", 500ms);
        auto response = promise->await();
        EXPECT_EQ(response.resolution, oregano::IResponsePromise::Resolution::Answer);
        EXPECT_EQ(response.message, "some response"s);
        EXPECT_THROW(promise->await(), oregano::IResponsePromise::ResultAlreadyReadException);
        EXPECT_THROW(
            promise->then([](const oregano::IResponsePromise::response_t&) {}), oregano::IResponsePromise::ResultAlreadyReadException);
    }

    {
        auto promise = request_sender->send_request("command_channel", "message", 500ms);
        std::timed_mutex mtx;
        mtx.lock();
        promise->then([&mtx](const oregano::IResponsePromise::response_t& p_response) {
            EXPECT_EQ(p_response.resolution, oregano::IResponsePromise::Resolution::Answer);
            EXPECT_EQ(p_response.message, "some response"s);
            mtx.unlock();
        });
        if (!mtx.try_lock_for(600ms)) {
            FAIL();
        }
        EXPECT_THROW(promise->await(), oregano::IResponsePromise::ResultAlreadyReadException);
        EXPECT_THROW(
            promise->then([](const oregano::IResponsePromise::response_t&) {}), oregano::IResponsePromise::ResultAlreadyReadException);
    }

    {
        auto promise = request_sender->send_request("wrong_command_channel1", "message", 500ms);
        auto response = promise->await();
        EXPECT_EQ(response.resolution, oregano::IResponsePromise::Resolution::Timeout);
        EXPECT_EQ(response.message, std::nullopt);
    }

    {
        std::timed_mutex mtx;
        mtx.lock();
        auto promise = request_sender->send_request("wrong_command_channel2", "message", 500ms);
        promise->then([&mtx](const oregano::IResponsePromise::response_t& p_response) {
            EXPECT_EQ(p_response.resolution, oregano::IResponsePromise::Resolution::Timeout);
            EXPECT_EQ(p_response.message, std::nullopt);
            mtx.unlock();
        });
        if (!mtx.try_lock_for(600ms)) {
            FAIL();
        }
    }

    {
        auto promise = request_sender->send_request("command_channel", "timeout", 500ms);
        auto response = promise->await();
        EXPECT_EQ(response.resolution, oregano::IResponsePromise::Resolution::Timeout);
        EXPECT_EQ(response.message, std::nullopt);

        std::this_thread::sleep_for(2s);
    }

    {
        std::timed_mutex mtx;
        mtx.lock();
        auto promise = request_sender->send_request("command_channel", "timeout", 500ms);
        promise->then([&mtx](const oregano::IResponsePromise::response_t& p_response) {
            EXPECT_EQ(p_response.resolution, oregano::IResponsePromise::Resolution::Timeout);
            EXPECT_EQ(p_response.message, std::nullopt);
            mtx.unlock();
        });
        if (!mtx.try_lock_for(600ms)) {
            FAIL();
        }

        std::this_thread::sleep_for(2s);
    }
}

TEST_F(TransactionTest, DistributedTest)
{
    using message_t = std::vector<std::pair<std::string, std::string>>;

    message_t subscriber1_messages;
    message_t subscriber2_messages;
    std::mutex subscriber1_sync;
    std::mutex subscriber2_sync;

    const auto handler_lambda = [&m_configuration = this->m_configuration](const std::string& p_handled_channel, std::mutex& sync) {
        auto broker = oregano::IMessageBrokerWrapper::create(*m_configuration);
        auto transaction_interface = oregano::ITransactionInterface::create(*broker);
        auto handler = transaction_interface->create_request_handler();

        handler->set_on_request_callback([&p_handled_channel](const std::string& p_channel, const std::string& p_message) {
            EXPECT_EQ(p_handled_channel, p_channel);
            return p_message;
        });

        handler->listen(p_handled_channel);

        std::lock_guard<std::mutex> g { sync };
    };

    subscriber1_sync.lock();
    subscriber2_sync.lock();

    std::thread t1(handler_lambda, "channel1", std::ref(subscriber1_sync));
    std::thread t2(handler_lambda, "channel2", std::ref(subscriber2_sync));

    std::this_thread::sleep_for(100ms);

    auto transaction_interface = oregano::ITransactionInterface::create(*m_broker);
    auto request_sender = transaction_interface->create_request_sender();

    auto response1 = request_sender->send_request("channel1", "message1", 500ms)->await();
    auto response2 = request_sender->send_request("channel2", "message2", 500ms)->await();

    subscriber1_sync.unlock();
    subscriber2_sync.unlock();

    if (t1.joinable()) {
        t1.join();
    }
    if (t2.joinable()) {
        t2.join();
    }

    EXPECT_EQ(response1.resolution, oregano::IResponsePromise::Resolution::Answer);
    EXPECT_EQ(response2.resolution, oregano::IResponsePromise::Resolution::Answer);

    EXPECT_EQ(response1.message, "message1"s);
    EXPECT_EQ(response2.message, "message2"s);
}
