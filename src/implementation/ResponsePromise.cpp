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

IResponsePromise::response_t ResponsePromise::await()
{
    if (m_result_read) {
        throw IResponsePromise::ResultAlreadyReadException("Result already read from the promise!");
    }
    if (m_then_applied) {
        throw IResponsePromise::ThenAlreadyAppliedException("Then callback already applied!");
    }
    if (m_timeout_mutex.try_lock_until(m_valid_until)) {
        m_result_read = true;
        return { IResponsePromise::Resolution::Answer, m_response };
    }
    m_result_read = true;
    return { IResponsePromise::Resolution::Timeout, std::nullopt };
}

void ResponsePromise::then(resolution_callback p_callback)
{
    if (m_result_read) {
        throw IResponsePromise::ResultAlreadyReadException("Result already read from the promise!");
    }
    if (m_then_applied) {
        throw IResponsePromise::ThenAlreadyAppliedException("Then callback already applied!");
    }
    m_then_thread = std::thread([this, p_callback]() {
        const auto resolution = await();
        p_callback(resolution);
    });
}

void ResponsePromise::on_response(const std::string& p_message)
{
    if (std::chrono::system_clock::now() > m_valid_until) {
        return;
    }
    m_response = p_message;
    m_timeout_mutex.unlock();
}

} // namespace oregano