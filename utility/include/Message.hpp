#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <memory>
#include <string>
#include "OrderType.h"

// Messages Type
enum class MessageType {
    ADD_ORDER,
    MODIFY_ORDER,
    CANCEL_ORDER
};

// Details for AddOrder
struct AddOrderDetails {
    std::string instrument;
    double price;
    int quantity;
    bool isBuy;
    OrderType type;

    AddOrderDetails(const std::string& instr, double p, int qty, bool buy, OrderType ordType)
        : instrument(instr), price(p), quantity(qty), isBuy(buy), type(ordType) {}
};

// Details for ModifyOrder
struct ModifyOrderDetails {
    unsigned int orderId;
    double newPrice;
    int newQuantity;

    ModifyOrderDetails(unsigned int id, double price, int qty)
        : orderId(id), newPrice(price), newQuantity(qty) {}
};

// Details for Cancel Order
struct CancelOrderDetails {
    unsigned int orderId;

    CancelOrderDetails(unsigned int id) : orderId(id) {}
};

// 定义消息结构体
struct Message {
    MessageType type;
    std::unique_ptr<AddOrderDetails> addOrderDetails;
    std::unique_ptr<ModifyOrderDetails> modifyDetails;
    std::unique_ptr<CancelOrderDetails> cancelDetails;

    // Factory methdos to create diofferent kinds of messages
    static Message createAddOrder(const std::string& instrument, double price, int quantity, bool isBuy, OrderType type) {
        Message msg;
        msg.type = MessageType::ADD_ORDER;
        msg.addOrderDetails = std::make_unique<AddOrderDetails>(instrument, price, quantity, isBuy, type);
        return msg;
    }

    static Message createModifyOrder(unsigned int orderId, double newPrice, int newQuantity) {
        Message msg;
        msg.type = MessageType::MODIFY_ORDER;
        msg.modifyDetails = std::make_unique<ModifyOrderDetails>(orderId, newPrice, newQuantity);
        return msg;
    }

    static Message createCancelOrder(unsigned int orderId) {
        Message msg;
        msg.type = MessageType::CANCEL_ORDER;
        msg.cancelDetails = std::make_unique<CancelOrderDetails>(orderId);
        return msg;
    }
};

#endif // MESSAGE_HPP
