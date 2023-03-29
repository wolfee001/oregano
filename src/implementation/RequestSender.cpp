#include "RequestSender.h"

#include "ResponsePromise.h"

#include <nlohmann/json.hpp>

#include <sole.hpp>

#ifdef _MSC_VER
#pragma warning(disable : 4702)
#endif
#include <cppcodec/base64_rfc4648.hpp>
#ifdef _MSC_VER
#pragma warning(default : 4702)
#endif

using namespace std::chrono_literals;

namespace oregano {

RequestSender::RequestSender(IMessageBroker& p_message_broker, const std::string& p_response_channel,
    const response_promise_register_callback& p_response_promise_register_callback,
    const response_promise_unregister_callback& p_response_promise_unregister_callback)
    : m_message_broker(p_message_broker)
    , m_response_channel(p_response_channel)
    , m_response_promise_register_callback { p_response_promise_register_callback }
    , m_response_promise_unregister_callback { p_response_promise_unregister_callback }
{
}

std::unique_ptr<IResponsePromise> RequestSender::send_request(
    const std::string& p_channel, const std::string& p_message, std::chrono::milliseconds p_timeout)
{
    const auto id = sole::uuid4().base62();
    nlohmann::json data;
    data["header"]["response_channel"] = m_response_channel;
    data["header"]["id"] = id;
    const auto valid_until = std::chrono::system_clock::now() + p_timeout;
    const auto valid_until_ms = std::chrono::duration_cast<std::chrono::milliseconds>(valid_until.time_since_epoch()).count();
    data["header"]["valid_until"] = valid_until_ms;
    data["payload"] = cppcodec::base64_rfc4648::encode(p_message);

    auto promise = std::make_unique<ResponsePromise>(
        [&id, &m_response_promise_unregister_callback = this->m_response_promise_unregister_callback]() {
            m_response_promise_unregister_callback(id);
        },
        valid_until);

    m_response_promise_register_callback(id, promise.get());

    m_message_broker.send_request(p_channel, data.dump());

    return promise;
}

} // namespace oregano