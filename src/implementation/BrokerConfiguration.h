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

private:
    std::string m_host;
    uint16_t m_port;
};

} // namespace oregano::broker_configuration