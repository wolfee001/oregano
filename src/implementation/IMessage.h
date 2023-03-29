#pragma once

#include <memory>
#include <string>

namespace oregano {

class MessageSerializationException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class IDictionary {
public:
    virtual ~IDictionary() = default;

    virtual void set_field(const std::string& p_name, const std::string& p_value) = 0;
    virtual const std::string& get_field(const std::string& p_name) = 0;

    virtual std::string serialize() = 0;
    virtual void deserialize(const std::string& p_data) = 0;

    static std::unique_ptr<IDictionary> create();
};

using IHeader = IDictionary;
using IPayload = IDictionary;

class IMessage {
public:
    virtual ~IMessage() = default;

    virtual void set_header(std::unique_ptr<IHeader> p_header) = 0;
    virtual IHeader& get_header() = 0;

    virtual void set_payload(const std::string& p_payload) = 0;
    virtual std::string& get_payload() = 0;

    virtual std::string serialize() = 0;
    virtual void deserialize(const std::string& p_data) = 0;

    static std::unique_ptr<IMessage> create();
};

} // namespace oregano