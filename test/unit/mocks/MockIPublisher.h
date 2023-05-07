#pragma once

#include <oregano/IPublisher.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace oregano {

class MockIPublisher : public IPublisher {
public:
    MOCK_METHOD(void, publish, (const std::string& p_channel, const std::string& p_message), (override));
};

} // namespace oregano
