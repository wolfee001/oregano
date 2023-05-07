#include <oregano/BrokerConfiguration.h>
#include <oregano/IMessageBrokerWrapper.h>
#include <oregano/ITransactionInterface.h>

#include <nlohmann/json.hpp>

#include <sole.hpp>

#include <iostream>
#include <thread>

int main()
{
    auto configuration = oregano::broker_configuration::Redis::create();
    configuration->set_host("127.0.0.1");
    configuration->set_port(6379);
    auto broker = oregano::IMessageBrokerWrapper::create(*configuration);

    auto transaction_interface = oregano::ITransactionInterface::create(*broker);

    auto handler = transaction_interface->create_request_handler();

    std::string id = sole::uuid4().base62();

    handler->set_on_request_callback([&id](const std::string& p_channel, const std::string& p_message) -> std::string {
        std::cout << "Receiving message on channel " << p_channel << std::endl;

        auto json = nlohmann::json::parse(p_message);
        std::cout << json.dump(2) << std::endl;

        auto response = nlohmann::json();
        response["id"] = id;
        response["response"] = json["payload"]["a"].get<uint32_t>() + json["payload"]["b"].get<uint32_t>();

        std::cout << "Responding" << std::endl;
        std::cout << response.dump(2) << std::endl << std::endl;

        return response.dump();
    });

    std::string channel = "example_transaction_channel";

    handler->listen(channel);

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}
