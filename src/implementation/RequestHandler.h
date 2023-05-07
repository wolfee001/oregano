#pragma once

#include <oregano/IPublisher.h>
#include <oregano/IRequestHandler.h>

#include "IMessageHandlerManager.h"

namespace oregano {

class RequestHandler : public IRequestHandler {
public:
    explicit RequestHandler(IMessageHandlerManager& p_message_handler_manager, IPublisher& p_publisher);

    void listen(const std::string& p_channel) override;
    void stop_listening(const std::string& p_channel) override;

    void set_on_request_callback(const on_request_callback& p_callback) override;

    void on_message(const std::string& p_channel, const std::string& p_message) override;

private:
    IMessageHandlerManager& m_message_handler_manager;
    IRequestHandler::on_request_callback m_on_request_callback = [](const std::string&, const std::string&) -> std::string { return {}; };
    IPublisher& m_publisher;
};

} // namespace oregano