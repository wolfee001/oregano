#include <oregano/BrokerConfiguration.h>
#include <oregano/IEventInterface.h>
#include <oregano/IMessageBrokerWrapper.h>

#include <nlohmann/json.hpp>

#include <iostream>
#include <thread>

int main()
{
    auto configuration = oregano::broker_configuration::Redis::create();
    configuration->set_host("127.0.0.1");
    configuration->set_port(6379);
    auto broker = oregano::IMessageBrokerWrapper::create(*configuration);

    auto event_interface = oregano::IEventInterface::create(*broker);

    auto subscriber = event_interface->create_subscriber();

    subscriber->set_on_message_callback([](const std::string& p_channel, const std::string& p_message) {
        std::cout << "Receiving message on channel " << p_channel << " at "
                  << std::to_string(
                         std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
                  << std::endl;

        auto json = nlohmann::json::parse(p_message);
        std::cout << json.dump(2) << std::endl << std::endl;
    });

    std::string channel = "example_event_channel";

    subscriber->subscribe(channel);

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}
