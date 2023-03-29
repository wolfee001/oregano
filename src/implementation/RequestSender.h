#pragma once

#include <oregano/IRequestSender.h>

#include "IMessageBroker.h"
#include "ResponsePromise.h"

#include <functional>

namespace oregano {

class RequestSender : public IRequestSender {
public:
    using response_promise_register_callback = std::function<void(const std::string& p_message_id, IResponsePromise* p_promise)>;
    using response_promise_unregister_callback = std::function<void(const std::string& p_message_id)>;

public:
    explicit RequestSender(IMessageBroker& p_message_broker, const std::string& p_response_channel,
        const response_promise_register_callback& p_response_promise_register_callback,
        const response_promise_unregister_callback& p_response_promise_unregister_callback);

    std::unique_ptr<IResponsePromise> send_request(
        const std::string& p_channel, const std::string& p_message, std::chrono::milliseconds p_timeout) override;

private:
    IMessageBroker& m_message_broker;
    std::string m_response_channel;
    response_promise_register_callback m_response_promise_register_callback;
    response_promise_unregister_callback m_response_promise_unregister_callback;
};

} // namespace oregano