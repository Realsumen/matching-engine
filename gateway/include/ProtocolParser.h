#ifndef PROTOCOL_PARSER_H
#define PROTOCOL_PARSER_H

#include <string>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include "Message.hpp"
#include "Trade.h"

class ProtocolParser {
public:
    static Message parse(const std::string& rawData, const std::string& protocol);

    static std::string serialize(const Message& message, const std::string& protocol);

private:
    ProtocolParser() = default;

    static Message parseTCP(const std::string& rawData);

    static std::string serializeTCPTrades(const std::vector<Trade>& trades);

};

#endif // PROTOCOL_PARSER_H
