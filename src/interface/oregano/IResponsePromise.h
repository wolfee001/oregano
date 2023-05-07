#pragma once

#include <chrono>
#include <functional>
#include <optional>
#include <string>

namespace oregano {

class IResponsePromise {
public:
    enum class Resolution { Answer, Timeout };

    struct response_t {
        IResponsePromise::Resolution resolution;
        std::optional<std::string> message;
    };

    class ResultAlreadyReadException : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    class ThenAlreadyAppliedException : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    using resolution_callback = std::function<void(const response_t& p_response)>;

    virtual ~IResponsePromise() = default;

    virtual response_t await() = 0;
    virtual void then(resolution_callback p_callback) = 0;
    virtual void on_response(const std::string& p_message) = 0;
};

} // namespace oregano