#include <oregano/BrokerConfiguration.h>
#include <oregano/IMessageBrokerWrapper.h>
#include <oregano/ITransactionInterface.h>

#include <sole.hpp>

#include <nlohmann/json.hpp>

#include <iostream>
#include <random>
#include <thread>

using namespace std::chrono_literals;

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

    auto transaction_interface = oregano::ITransactionInterface::create(*broker);

    auto request_sender = transaction_interface->create_request_sender();

    std::string channel = "example_transaction_channel";

    while (true) {
        nlohmann::json json;
        json["id"] = id;
        json["payload"]["a"] = dist(rng);
        json["payload"]["b"] = dist(rng);

        std::cout << "Sending request on channel " << channel << std::endl;
        std::cout << json.dump(2) << std::endl;

        // Example with synchronous await
        auto response = request_sender->send_request(channel, json.dump(), 1s)->await();
        if (response.resolution == oregano::IResponsePromise::Resolution::Timeout) {
            std::cout << "REQUEST MESSAGE TIMED OUT" << std::endl;
            continue;
        }

        std::cout << "Response" << std::endl;
        std::cout << nlohmann::json::parse(*(response.message)).dump(2) << std::endl;

        // Example with then callback
        std::mutex mtx;
        mtx.lock();
        request_sender->send_request(channel, json.dump(), 1s)->then([&mtx](oregano::IResponsePromise::response_t p_response) {
            if (p_response.resolution == oregano::IResponsePromise::Resolution::Timeout) {
                std::cout << "REQUEST MESSAGE TIMED OUT" << std::endl << std::endl;
                mtx.unlock();
                return;
            }

            std::cout << "Response" << std::endl;
            std::cout << nlohmann::json::parse(*(p_response.message)).dump(2) << std::endl << std::endl;
            mtx.unlock();
        });
        mtx.lock();

        std::this_thread::sleep_for(std::chrono::seconds(dist(rng)));
    }

    return 0;
}
