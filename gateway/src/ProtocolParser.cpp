#include "ProtocolParser.h"
#include <stdexcept>
#include <iostream>
#include <rapidjson/document.h>

using namespace rapidjson;

auto ProtocolParser::parse(const std::string& rawData, const std::string& protocol) -> Message
{
    if (protocol == "TCP")
    {
        return parseTCP(rawData);
    }
    
    throw std::invalid_argument("Unsupported protocol" + protocol);
}

auto ProtocolParser::serialize(const Message& trades, const std::string& protocol) -> std::string 
{   
    if (protocol == "TCP")
    {   
        // TODO: design serialization: what messages does the front ends need to receive?
    }

    throw std::invalid_argument("Unsupported protocol: " + protocol);
}

auto ProtocolParser::parseTCP(const std::string& rawData) -> Message
{
    Document document;
    if (document.Parse(rawData.c_str()).HasParseError()) {
        throw std::invalid_argument("JSON parsing error in TCP data");
    }

    if (!document.IsObject()) {
        throw std::invalid_argument("Invalid JSON format: not an object");
    }

    if (!document.HasMember("type") || !document["type"].IsString()) {
        throw std::invalid_argument("Missing or invalid 'type' field in TCP data");
    }

    std::string typeStr = document["type"].GetString();

    if (typeStr == "ADD_ORDER")
    {
        if (!document.HasMember("instrument") || !document["instrument"].IsString() ||
            !document.HasMember("price") || !document["price"].IsNumber() ||
            !document.HasMember("quantity") || !document["quantity"].IsInt() ||
            !document.HasMember("isBuy") || !document["isBuy"].IsBool() ||
            !document.HasMember("orderType") || !document["orderType"].IsString()) {
            throw std::invalid_argument("Missing required fields for ADD_ORDER in TCP data");
        }

        std::string instrument = document["instrument"].GetString();
        double price = document["price"].GetDouble();
        int quantity = document["quantity"].GetInt();
        bool isBuy = document["isBuy"].GetBool();
        std::string orderTypeStr = document["orderType"].GetString();

        static const std::unordered_map<std::string, OrderType> orderTypeMap = {
            {"LIMIT", OrderType::LIMIT},
            {"MARKET", OrderType::MARKET},
            {"STOP", OrderType::STOP}
        };

        auto iter = orderTypeMap.find(orderTypeStr);
        if (iter == orderTypeMap.end()) {
            throw std::invalid_argument("Unsupported order type: " + orderTypeStr);
        }

        return Message::createAddOrderMessage(instrument, price, quantity, isBuy, iter->second);
    }
    if (typeStr == "MODIFY_ORDER") {
        if (!document.HasMember("orderId") || !document["orderId"].IsUint() ||
            !document.HasMember("instrument") || !document["instrument"].IsString() ||
            !document.HasMember("newPrice") || !document["newPrice"].IsNumber() ||
            !document.HasMember("newQuantity") || !document["newQuantity"].IsInt()) {
            throw std::invalid_argument("Missing required fields for MODIFY_ORDER in TCP data");
        }

        unsigned int orderId = document["orderId"].GetUint();
        std::string instrument = document["instrument"].GetString();
        double newPrice = document["newPrice"].GetDouble();
        int newQuantity = document["newQuantity"].GetInt();

        return Message::createModifyOrderMessage(orderId, instrument, newPrice, newQuantity);
    }
    if (typeStr == "CANCEL_ORDER") {
        if (!document.HasMember("orderId") || !document["orderId"].IsUint() ||
            !document.HasMember("instrument") || !document["instrument"].IsString()) {
            throw std::invalid_argument("Missing required fields for CANCEL_ORDER in TCP data");
        }

        unsigned int orderId = document["orderId"].GetUint();
        std::string instrument = document["instrument"].GetString();

        return Message::createCancelOrderMessage(orderId, instrument);
    }
    throw std::invalid_argument("Unsupported message type: " + typeStr);
}

auto ProtocolParser::serializeTCPTrades(const std::vector<Trade> &trades) -> std::string
{
    // TODO serialize trades and send to the front end
    return {};
}
