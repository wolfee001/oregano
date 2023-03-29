#pragma once

#include <oregano/IResponsePromise.h>

#include <mutex>
#include <thread>

namespace oregano {

class ResponsePromise : public IResponsePromise {
public:
    using response_promise_unregister_callback = std::function<void()>;

public:
    ResponsePromise(const response_promise_unregister_callback& p_response_promise_unregister_callback,
        std::chrono::system_clock::time_point p_valid_until);
    ~ResponsePromise();

    std::pair<IResponsePromise::Resolution, std::string> await() override;
    void then(resolution_callback p_callback) override;
    void on_response(const std::string& p_message) override;

private:
    response_promise_unregister_callback m_response_promise_unregister_callback;
    std::string m_response;
    std::chrono::system_clock::time_point m_valid_until;
    std::timed_mutex m_timeout_mutex;
    std::thread m_then_thread;
};

} // namespace oregano