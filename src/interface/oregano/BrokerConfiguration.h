#pragma once

#include <memory>
#include <string>
#include <variant>

namespace oregano::broker_configuration {

/**
 * Class for configuration of Redis broker
 */
class Redis {
public:
    virtual ~Redis() = default;

    /**
     * Sets the hostname
     *
     * @param p_host the hostname
     */
    virtual void set_host(const std::string& p_host) = 0;

    /**
     * Returns the hostname
     *
     * @return the hostname
     */
    virtual const std::string& get_host() const = 0;

    /**
     * Sets the port
     *
     * @param p_port the port
     */
    virtual void set_port(uint16_t p_port) = 0;

    /**
     * Returns the port
     *
     * @return the port
     */
    virtual uint16_t get_port() const = 0;

    /**
     * Sets the user
     *
     * @param p_user the username
     */
    virtual void set_user(const std::string& p_user) = 0;

    /**
     * Returns the username
     *
     * @return the username
     */
    virtual const std::string& get_user() = 0;

    /**
     * Sets the password
     *
     * @param p_password the password
     */
    virtual void set_password(const std::string& p_password) = 0;

    /**
     * Returns the password
     *
     * @return the password
     */
    virtual const std::string& get_password() = 0;

    /**
     * Creates an instance
     *
     * @return a unique_ptr to an instance
     */
    static std::unique_ptr<Redis> create();
};

/**
 * Variant type to hold all broker configuration
 */
using Configuration = std::variant<std::reference_wrapper<Redis>>;

} // namespace oregano::broker_configuration