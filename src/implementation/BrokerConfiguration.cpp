#include "BrokerConfiguration.h"

namespace oregano::broker_configuration {

RedisImpl::RedisImpl()
    : m_port(0)
{
}

void RedisImpl::set_host(const std::string& p_host) { m_host = p_host; }

const std::string& RedisImpl::get_host() const { return m_host; }

void RedisImpl::set_port(uint16_t p_port) { m_port = p_port; }

uint16_t RedisImpl::get_port() const { return m_port; }

void RedisImpl::set_user(const std::string& p_user) { m_user = p_user; }

const std::string& RedisImpl::get_user() { return m_user; }

void RedisImpl::set_password(const std::string& p_password) { m_password = p_password; }

const std::string& RedisImpl::get_password() { return m_password; }

std::unique_ptr<Redis> Redis::create() { return std::make_unique<RedisImpl>(); }

} // namespace oregano::broker_configuration