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
            return std::make_unique<RedisMessageBroker>(p_config.get().get_host(), p_config.get().get_port(), sole::uuid4().base62());
        }
    };

    return std::visit(BrokerCreators {}, p_configuration);
}

} // namespace oregano