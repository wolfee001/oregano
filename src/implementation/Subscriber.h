#pragma once

#include <oregano/ISubscriber.h>

#include "IMessageHandlerManager.h"

namespace oregano {

class Subscriber : public ISubscriber {
public:
    explicit Subscriber(IMessageHandlerManager& p_message_handler_manager);
    ~Subscriber() override;

    void subscribe(const std::string& p_channel) override;
    void unsubscribe(const std::string& p_channel) override;

    void set_on_message_callback(const on_message_callback& p_callback) override;

    void on_message(const std::string& p_channel, const std::string& p_message) override;

private:
    IMessageHandlerManager& m_message_handler_manager;
    ISubscriber::on_message_callback m_on_message_callback = [](const std::string&, const std::string&) {};
};

} // namespace oregano