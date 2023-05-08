#include "IMessageBroker.h"

#include "BrokerConfiguration.h"
#include "MessageBroker.h"
#include "RedisBrokerConnector.h"

#include <sole.hpp>

#include <memory>

namespace oregano {

std::unique_ptr<IMessageBroker> IMessageBroker::create(broker_configuration::Configuration p_configuration)
{
    struct BrokerCreators {
        std::unique_ptr<IMessageBroker> operator()(std::reference_wrapper<broker_configuration::Redis> p_config)
        {
            auto config = std::make_unique<broker_configuration::RedisImpl>();
            config->set_host(p_config.get().get_host());
            config->set_port(p_config.get().get_port());
            config->set_user(p_config.get().get_user());
            config->set_password(p_config.get().get_password());
            std::unique_ptr<IMessageBrokerConnector> redis
                = std::make_unique<RedisBrokerConnector>(std::move(config), sole::uuid4().base62());
            auto broker = std::make_unique<MessageBroker>(std::move(redis));
            return broker;
        }
    };

    return std::visit(BrokerCreators {}, p_configuration);
}

} // namespace oregano