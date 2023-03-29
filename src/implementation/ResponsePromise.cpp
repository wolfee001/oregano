#include "ResponsePromise.h"

namespace oregano {

ResponsePromise::ResponsePromise(
    const response_promise_unregister_callback& p_response_promise_unregister_callback, std::chrono::system_clock::time_point p_valid_until)
    : m_response_promise_unregister_callback { p_response_promise_unregister_callback }
    , m_valid_until(p_valid_until)
{
    m_timeout_mutex.lock();
}

ResponsePromise::~ResponsePromise()
{
    if (m_then_thread.joinable()) {
        m_then_thread.join();
    }
    m_response_promise_unregister_callback();
}

std::pair<IResponsePromise::Resolution, std::string> ResponsePromise::await()
{
    if (m_timeout_mutex.try_lock_until(m_valid_until)) {
        return { IResponsePromise::Resolution::Answer, m_response };
    }
    return { IResponsePromise::Resolution::Timeout, {} };
}

void ResponsePromise::then(resolution_callback p_callback)
{
    m_then_thread = std::thread([this, &p_callback]() {
        const auto resolution = await();
        p_callback(resolution.first, resolution.second);
    });
}

void ResponsePromise::on_response(const std::string& p_message)
{
    m_response = p_message;
    m_timeout_mutex.unlock();
}

} // namespace oregano