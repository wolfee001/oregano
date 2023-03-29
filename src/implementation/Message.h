#pragma once

#include <string>
#include <unordered_map>

namespace oregano {

class MessageSerializationException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class Header {
public:
    void set_field(const std::string& p_name, const std::string& p_value);
    const std::string& get_field(const std::string& p_name);

    std::string serialize();
    void deserialize(const std::string& p_data);

private:
    std::unordered_map<std::string, std::string> m_dictionay;
};

class Message {
public:
    void set_header(const Header& p_header);
    Header& get_header();

    void set_payload(const std::string& p_payload);
    std::string& get_payload();

    std::string serialize();
    void deserialize(const std::string& p_data);

private:
    Header m_header;
    std::string m_payload;
};

} // namespace oregano