#pragma once

#include <chrono>
#include <functional>
#include <string>

namespace oregano {

class IResponsePromise {
public:
    enum class Resolution { Answer,
        Timeout };

    using resolution_callback = std::function<void(Resolution p_resolution, const std::string& p_message)>;

    virtual ~IResponsePromise() = default;

    virtual std::pair<IResponsePromise::Resolution, std::string> await() = 0;
    virtual void then(resolution_callback p_callback) = 0;
    virtual void on_response(const std::string& p_message) = 0;
};

} // namespace oregano