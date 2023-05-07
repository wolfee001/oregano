#include "IMessageBroker.h"

#include "RedisMessageBroker.h"

#include <sole.hpp>

#include <memory>

namespace oregano {

std::unique_ptr<IMessageBroker> IMessageBroker::create(broker_configuration::Configuration p_configuration)
{
    struct BrokerCreators {
        std::unique_ptr<IMessageBroker> operator()(std::reference_wrapper<broker_configuration::Redis> p_config)
        {
            auto redis = std::make_unique<RedisClass>(p_config.get().get_host(), p_config.get().get_port(), sole::uuid4().base62());
            auto broker = std::make_unique<RedisMessageBroker>(std::move(redis));
            return broker;
        }
    };

    return std::visit(BrokerCreators {}, p_configuration);
}

} // namespace oregano