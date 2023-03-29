#include "Message.h"

#include <nlohmann/json.hpp>

namespace oregano {

void Header::set_field(const std::string& p_name, const std::string& p_value) { m_dictionay[p_name] = p_value; }

const std::string& Header::get_field(const std::string& p_name) { return m_dictionay.at(p_name); }

std::string Header::serialize()
{
    nlohmann::json json;
    for (auto& [name, value] : m_dictionay) {
        json[name] = value;
    }
    return json.dump();
}

void Header::deserialize(const std::string& p_data)
{
    nlohmann::json json = nlohmann::json::parse(p_data);

    for (auto element = json.begin(); element != json.end(); ++element) {
        if (!element.value().is_string()) {
            throw MessageSerializationException("Value is not a string in the encoded message header!");
        }
        set_field(element.key(), element.value());
    }
}

void Message::set_header(const Header& p_header) { m_header = p_header; }

Header& Message::get_header() { return m_header; }

void Message::set_payload(const std::string& p_payload) { m_payload = p_payload; }

std::string& Message::get_payload() { return m_payload; }

std::string Message::serialize()
{
    nlohmann::json json;
    json["header"] = nlohmann::json::parse(m_header.serialize());
    json["payload"] = m_payload;
    return json.dump();
}

void Message::deserialize(const std::string& p_data)
{
    nlohmann::json json = nlohmann::json::parse(p_data);

    if (!json.contains("header")) {
        throw MessageSerializationException("Message has no header!");
    }
    auto const header = json["header"];
    if (!header.is_object()) {
        throw MessageSerializationException("Header is not an object!");
    }
    m_header.deserialize(header.dump());

    if (!json.contains("payload")) {
        throw MessageSerializationException("Message has no payload!");
    }
    auto const payload = json["payload"];
    if (!payload.is_string()) {
        throw MessageSerializationException("Payload is not a string!");
    }
    m_payload = payload;
}

} // namespace oregano