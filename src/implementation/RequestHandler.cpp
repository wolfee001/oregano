#include "RequestHandler.h"

#include <nlohmann/json.hpp>

#ifdef _MSC_VER
#pragma warning(disable : 4702)
#endif
#include <cppcodec/base64_rfc4648.hpp>
#ifdef _MSC_VER
#pragma warning(default : 4702)
#endif

namespace oregano {

RequestHandler::RequestHandler(IMessageHandlerManager& p_message_handler_manager, IPublisher& p_publisher)
    : m_message_handler_manager(p_message_handler_manager)
    , m_publisher(p_publisher)
{
}

void RequestHandler::listen(const std::string& p_channel) { m_message_handler_manager.add_message_handler(p_channel, *this); }

void RequestHandler::stop_listening(const std::string& p_channel) { m_message_handler_manager.remove_message_handler(p_channel, *this); }

void RequestHandler::set_on_request_callback(const on_request_callback& p_callback) { m_on_request_callback = p_callback; }

void RequestHandler::on_message(const std::string& p_channel, const std::string& p_message)
{
    const auto message = nlohmann::json::parse(p_message);

    if (message["header"].contains("valid_until")) {
        const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        const auto valid_until = std::chrono::milliseconds(message["header"]["valid_until"]);
        if (now > valid_until) {
            return;
        }
    }

    const std::string message_id = message["header"]["id"];
    const std::string response_channel = message["header"]["response_channel"];
    const std::string message_payload = message["payload"];

    const auto response_payload = m_on_request_callback(p_channel, cppcodec::base64_rfc4648::decode<std::string>(message_payload));

    auto response = nlohmann::json();
    response["header"]["id"] = message_id;
    response["payload"] = cppcodec::base64_rfc4648::encode(response_payload);

    m_publisher.publish(response_channel, response.dump());
}

} // namespace oregano