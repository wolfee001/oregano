#include "TransactionInterface.h"

#include "EventInterface.h"
#include "IMessageBroker.h"
#include "MessageHandlerManager.h"
#include "RequestHandler.h"
#include "RequestSender.h"
#include "Subscriber.h"

#include <sole.hpp>

#include <nlohmann/json.hpp>

#ifdef _MSC_VER
#pragma warning(disable : 4702)
#endif
#include <cppcodec/base64_rfc4648.hpp>
#ifdef _MSC_VER
#pragma warning(default : 4702)
#endif

namespace oregano {

TransactionInterface::TransactionInterface(IMessageBrokerWrapper& p_message_broker,
    std::unique_ptr<IMessageHandlerManager> p_message_handler_manager, std::unique_ptr<IEventInterface> p_event_interface)
    : m_message_broker(p_message_broker)
    , m_message_handler_manager(std::move(p_message_handler_manager))
    , m_event_interface(std::move(p_event_interface))
    , m_response_channel(sole::uuid4().base62())
    , m_publisher(m_event_interface->create_publisher())
    , m_subscriber(m_event_interface->create_subscriber())
{
    m_callback_register_id = m_message_broker.get_message_broker().register_transaction_callback(
        [&m_message_handler_manager = this->m_message_handler_manager](
            const std::string& p_channel, const std::string& p_message) { m_message_handler_manager->on_message(p_channel, p_message); });

    m_subscriber->set_on_message_callback(
        [&m_on_response_callbacks_lock = this->m_on_response_callbacks_lock, &m_on_response_callbacks = this->m_on_response_callbacks](
            const std::string&, const std::string& p_message) {
            const auto message = nlohmann::json().parse(p_message);
            const std::string id = message["header"]["id"];
            const std::string payload = message["payload"];

            std::lock_guard<std::mutex> guard { m_on_response_callbacks_lock };

            if (auto it = m_on_response_callbacks.find(id); it != m_on_response_callbacks.end()) {
                it->second->on_response(cppcodec::base64_rfc4648::decode<std::string>(payload));
            }
        });

    m_subscriber->subscribe(m_response_channel);
}

TransactionInterface::~TransactionInterface() { m_message_broker.get_message_broker().remove_transaction_callback(m_callback_register_id); }

std::unique_ptr<IRequestSender> TransactionInterface::create_request_sender()
{
    return std::make_unique<RequestSender>(
        m_message_broker.get_message_broker(), m_response_channel,
        [&m_on_response_callbacks_lock = this->m_on_response_callbacks_lock, &m_on_response_callbacks = this->m_on_response_callbacks](
            const std::string& p_message_id, IResponsePromise* p_promise) {
            std::lock_guard<std::mutex> guard { m_on_response_callbacks_lock };
            m_on_response_callbacks[p_message_id] = p_promise;
        },
        [&m_on_response_callbacks_lock = this->m_on_response_callbacks_lock, &m_on_response_callbacks = this->m_on_response_callbacks](
            const std::string& p_message_id) {
            std::lock_guard<std::mutex> guard { m_on_response_callbacks_lock };
            m_on_response_callbacks.erase(p_message_id);
        });
}

std::unique_ptr<IRequestHandler> TransactionInterface::create_request_handler()
{
    return std::make_unique<RequestHander>(*m_message_handler_manager, *m_publisher);
}

std::unique_ptr<ITransactionInterface> ITransactionInterface::create(IMessageBrokerWrapper& p_message_broker)
{
    return std::make_unique<TransactionInterface>(p_message_broker,
        std::make_unique<MessageHandlerManager>(MessageHandlerManager::Type::Transaction, p_message_broker),
        std::make_unique<EventInterface>(
            p_message_broker, std::make_unique<MessageHandlerManager>(MessageHandlerManager::Type::Event, p_message_broker)));
}

} // namespace oregano
