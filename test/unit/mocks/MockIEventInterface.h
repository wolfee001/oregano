#pragma once

#include <oregano/IEventInterface.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace oregano {

class MockIEventInterface : public IEventInterface {
public:
    MOCK_METHOD(std::unique_ptr<IPublisher>, create_publisher, (), (override));
    MOCK_METHOD(std::unique_ptr<ISubscriber>, create_subscriber, (), (override));
};

} // namespace oregano
