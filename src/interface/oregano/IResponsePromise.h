#pragma once

#include <chrono>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>

namespace oregano {

/**
 * @brief Class to describe a response that will happen in the future
 *
 * This is the deferred response facility of the transaction system
 */
class IResponsePromise {
public:
    /**
     * Possible promise resolution types
     */
    enum class Resolution {
        Answer, ///< The promise result in an answer
        Timeout, ///< The promise timed out
    };

    /**
     * Response type
     */
    struct response_t {
        IResponsePromise::Resolution resolution; ///< The type of reolution
        std::optional<std::string> message; ///< Optional response message. Must be valid if the resolution is Answer.
    };

    /**
     * Exception to indicate the second read of the response
     */
    class ResultAlreadyReadException : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    /**
     * Exception to indicate the second 'then' on the promise
     */
    class ThenAlreadyAppliedException : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    /**
     * Callback type for the 'then' function
     *
     * @param p_response the response itself
     */
    using resolution_callback = std::function<void(response_t p_response)>;

    virtual ~IResponsePromise() = default;

    /**
     * Waits for the resolution of the response synchronously
     *
     * @return the reponse of the promise
     */
    virtual response_t await() = 0;

    /**
     * Sets the callback for async promise resolution
     *
     * @param p_callback the function to call on the resolution
     */
    virtual void then(resolution_callback p_callback) = 0;

    /**
     * Injects a response into the promise. Must not be used from outside.
     */
    virtual void on_response(const std::string& p_message) = 0;
};

} // namespace oregano