#pragma once

#include <memory>
#include <string>
#include <variant>

namespace oregano::broker_configuration {

class Redis {
public:
    virtual ~Redis() = default;

    virtual void set_host(const std::string& p_host) = 0;
    virtual const std::string& get_host() const = 0;

    virtual void set_port(uint16_t p_port) = 0;
    virtual uint16_t get_port() const = 0;

    static std::unique_ptr<Redis> create();
};

using Configuration = std::variant<std::reference_wrapper<Redis>>;

} // namespace oregano::broker_configuration