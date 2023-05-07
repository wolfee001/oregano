#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <oregano/BrokerConfiguration.h>
#include <oregano/IEventInterface.h>
#include <oregano/IMessageBrokerWrapper.h>
#include <oregano/ITransactionInterface.h>

#include <chrono>
#include <mutex>
#include <thread>

using namespace std::chrono_literals;
using namespace std::string_literals;

class PubSubTest : public ::testing::Test {
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

TEST_F(PubSubTest, Test)
{
    auto event_interface = oregano::IEventInterface::create(*m_broker);

    auto publisher = event_interface->create_publisher();
    auto subscriber1 = event_interface->create_subscriber();
    auto subscriber2 = event_interface->create_subscriber();

    using message_t = std::vector<std::pair<std::string, std::string>>;

    message_t subscriber1_messages;
    message_t subscriber2_messages;

    subscriber1->set_on_message_callback([&subscriber1_messages](const std::string& p_channel, const std::string& p_message) {
        subscriber1_messages.emplace_back(p_channel, p_message);
    });

    subscriber2->set_on_message_callback([&subscriber2_messages](const std::string& p_channel, const std::string& p_message) {
        subscriber2_messages.emplace_back(p_channel, p_message);
    });

    subscriber1->subscribe("some_channel1");
    subscriber1->subscribe("some_channel2");
    subscriber2->subscribe("some_channel1");

    {
        publisher->publish("some_channel1", "UniqueMessage1");
        publisher->publish("some_channel2", "UniqueMessage2");
        std::this_thread::sleep_for(10ms);

        EXPECT_THAT(subscriber1_messages,
            testing::ElementsAre(std::make_pair("some_channel1", "UniqueMessage1"), std::make_pair("some_channel2", "UniqueMessage2")));
        EXPECT_THAT(subscriber2_messages, testing::ElementsAre(std::make_pair("some_channel1", "UniqueMessage1")));
    }

    subscriber1->unsubscribe("some_channel2");

    {
        publisher->publish("some_channel1", "UniqueMessage3");
        publisher->publish("some_channel2", "UniqueMessage4");
        std::this_thread::sleep_for(10ms);

        EXPECT_THAT(subscriber1_messages,
            testing::ElementsAre(std::make_pair("some_channel1", "UniqueMessage1"), std::make_pair("some_channel2", "UniqueMessage2"),
                std::make_pair("some_channel1", "UniqueMessage3")));
        EXPECT_THAT(subscriber2_messages,
            testing::ElementsAre(std::make_pair("some_channel1", "UniqueMessage1"), std::make_pair("some_channel1", "UniqueMessage3")));
    }

    subscriber1->subscribe("some_channel2");

    {
        publisher->publish("some_channel1", "UniqueMessage5");
        publisher->publish("some_channel2", "UniqueMessage6");
        std::this_thread::sleep_for(10ms);

        EXPECT_THAT(subscriber1_messages,
            testing::ElementsAre(std::make_pair("some_channel1", "UniqueMessage1"), std::make_pair("some_channel2", "UniqueMessage2"),
                std::make_pair("some_channel1", "UniqueMessage3"), std::make_pair("some_channel1", "UniqueMessage5"),
                std::make_pair("some_channel2", "UniqueMessage6")));
        EXPECT_THAT(subscriber2_messages,
            testing::ElementsAre(std::make_pair("some_channel1", "UniqueMessage1"), std::make_pair("some_channel1", "UniqueMessage3"),
                std::make_pair("some_channel1", "UniqueMessage5")));
    }

    subscriber1.reset(nullptr);

    {
        publisher->publish("some_channel1", "UniqueMessage7");
        publisher->publish("some_channel2", "UniqueMessage8");
        std::this_thread::sleep_for(10ms);

        EXPECT_THAT(subscriber1_messages,
            testing::ElementsAre(std::make_pair("some_channel1", "UniqueMessage1"), std::make_pair("some_channel2", "UniqueMessage2"),
                std::make_pair("some_channel1", "UniqueMessage3"), std::make_pair("some_channel1", "UniqueMessage5"),
                std::make_pair("some_channel2", "UniqueMessage6")));
        EXPECT_THAT(subscriber2_messages,
            testing::ElementsAre(std::make_pair("some_channel1", "UniqueMessage1"), std::make_pair("some_channel1", "UniqueMessage3"),
                std::make_pair("some_channel1", "UniqueMessage5"), std::make_pair("some_channel1", "UniqueMessage7")));
    }
}

TEST_F(PubSubTest, DistributedTest)
{
    using message_t = std::vector<std::pair<std::string, std::string>>;

    message_t subscriber1_messages;
    message_t subscriber2_messages;
    message_t subscriber3_messages;
    std::mutex subscriber1_sync;
    std::mutex subscriber2_sync;
    std::mutex subscriber3_sync;

    const auto subscriber_lambda = [&m_configuration = this->m_configuration](
                                       const std::vector<std::string>& p_channels, message_t& p_message_collector, std::mutex& sync) {
        auto broker = oregano::IMessageBrokerWrapper::create(*m_configuration);
        auto event_interface = oregano::IEventInterface::create(*broker);
        auto subscriber = event_interface->create_subscriber();

        subscriber->set_on_message_callback([&p_message_collector](const std::string& p_channel, const std::string& p_message) {
            p_message_collector.emplace_back(p_channel, p_message);
        });

        for (const auto& element : p_channels) {
            subscriber->subscribe(element);
        }

        std::lock_guard<std::mutex> g { sync };
    };

    subscriber1_sync.lock();
    subscriber2_sync.lock();
    subscriber3_sync.lock();

    std::thread t1(subscriber_lambda, std::vector<std::string> { "channel1" }, std::ref(subscriber1_messages), std::ref(subscriber1_sync));
    std::thread t2(
        subscriber_lambda, std::vector<std::string> { "channel1", "channel2" }, std::ref(subscriber2_messages), std::ref(subscriber2_sync));
    std::thread t3(subscriber_lambda, std::vector<std::string> { "channel2" }, std::ref(subscriber3_messages), std::ref(subscriber3_sync));

    std::this_thread::sleep_for(100ms);

    auto event_interface = oregano::IEventInterface::create(*m_broker);
    auto publisher = event_interface->create_publisher();
    publisher->publish("channel1", "message1");
    publisher->publish("channel2", "message2");

    std::this_thread::sleep_for(100ms);

    subscriber1_sync.unlock();
    subscriber2_sync.unlock();
    subscriber3_sync.unlock();

    if (t1.joinable()) {
        t1.join();
    }
    if (t2.joinable()) {
        t2.join();
    }
    if (t3.joinable()) {
        t3.join();
    }

    EXPECT_THAT(subscriber1_messages, testing::ElementsAre(std::make_pair("channel1", "message1")));
    EXPECT_THAT(subscriber2_messages, testing::ElementsAre(std::make_pair("channel1", "message1"), std::make_pair("channel2", "message2")));
    EXPECT_THAT(subscriber3_messages, testing::ElementsAre(std::make_pair("channel2", "message2")));
}
