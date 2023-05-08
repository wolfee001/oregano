#pragma once

#include <oregano/BrokerConfiguration.h>

namespace oregano::broker_configuration {

class RedisImpl : public Redis {
public:
    RedisImpl();

    void set_host(const std::string& p_host) override;
    const std::string& get_host() const override;

    void set_port(uint16_t p_port) override;
    uint16_t get_port() const override;

    void set_user(const std::string& p_user) override;
    const std::string& get_user() override;

    void set_password(const std::string& p_password) override;
    const std::string& get_password() override;

private:
    std::string m_host;
    uint16_t m_port;
    std::string m_user { "default" };
    std::string m_password;
};

} // namespace oregano::broker_configuration