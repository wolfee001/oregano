#include "BrokerConfiguration.h"

namespace oregano::broker_configuration {

void RedisImpl::set_host(const std::string& p_host) { m_host = p_host; }

const std::string& RedisImpl::get_host() const { return m_host; }

void RedisImpl::set_port(uint16_t p_port) { m_port = p_port; }

uint16_t RedisImpl::get_port() const { return m_port; }

std::unique_ptr<Redis> Redis::create() { return std::make_unique<RedisImpl>(); }

} // namespace oregano::broker_configuration