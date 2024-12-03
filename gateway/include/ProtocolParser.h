#ifndef PROTOCOL_PARSER_H
#define PROTOCOL_PARSER_H

#include <string>
#include "Message.hpp"
#include "Trade.h"

class ProtocolParser {
public:
    static auto parse(const std::string& rawData, const std::string& protocol) -> Message;

    static auto serialize(const Message& message, const std::string& protocol) -> std::string;

private:
    ProtocolParser() = default;

    static auto parseTCP(const std::string& rawData) -> Message;

    static auto serializeTCPTrades(const std::vector<Trade>& trades) -> std::string;

};

#endif // PROTOCOL_PARSER_H
