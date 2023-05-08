#include <oregano/BrokerConfiguration.h>
#include <oregano/IEventInterface.h>
#include <oregano/IMessageBrokerWrapper.h>

#include <sole.hpp>

#include <nlohmann/json.hpp>

#include <iostream>
#include <random>
#include <thread>

int main()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1, 10);

    std::string id = sole::uuid4().base62();

    auto configuration = oregano::broker_configuration::Redis::create();
    configuration->set_host("127.0.0.1");
    configuration->set_port(6379);
    auto broker = oregano::IMessageBrokerWrapper::create(*configuration);

    auto event_interface = oregano::IEventInterface::create(*broker);

    auto publisher = event_interface->create_publisher();

    int count = 0;

    std::string channel = "example_event_channel";

    while (true) {
        nlohmann::json json;
        json["id"] = id;
        json["payload"]["timestamp"] = std::to_string(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
        json["payload"]["message_count"] = count;

        count++;

        std::cout << "Publishing message on channel " << channel << std::endl;
        std::cout << json.dump(2) << std::endl << std::endl;

        publisher->publish(channel, json.dump());

        std::this_thread::sleep_for(std::chrono::seconds(dist(rng)));
    }

    return 0;
}
