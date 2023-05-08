#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../src/implementation/BrokerConfiguration.h"

using namespace std::string_literals;

class BrokerConfigurationTest : public ::testing::Test {
protected:
    oregano::broker_configuration::RedisImpl m_redis;
};

TEST_F(BrokerConfigurationTest, GIVEN_RedisImpl_WHEN_SetUp_THEN_DataStored)
{
    EXPECT_EQ(m_redis.get_host(), ""s);
    EXPECT_EQ(m_redis.get_port(), 0);

    m_redis.set_host("host");
    m_redis.set_port(1234);

    EXPECT_EQ(m_redis.get_host(), "host"s);
    EXPECT_EQ(m_redis.get_port(), 1234);
}

TEST_F(BrokerConfigurationTest, GIVEN_Factory_WHEN_CreateRedisData_THEN_CorrespondingClassCreated)
{
    EXPECT_NE(oregano::broker_configuration::Redis::create(), std::unique_ptr<oregano::broker_configuration::Redis> {});
}
