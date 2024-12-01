#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <memory>
#include <string>
#include "OrderType.h"
#include "TimestampUtility.h"

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

    AddOrderDetails(const std::string& instr, double p, int qty, bool buy, OrderType orderType)
        : instrument(instr), price(p), quantity(qty), isBuy(buy), type(orderType) {}
};

// Details for ModifyOrder
struct ModifyOrderDetails {
    unsigned int orderId;
    std::string instrument;
    double newPrice;
    int newQuantity;

    ModifyOrderDetails(unsigned int id, std::string instrument, double price, int qty)
        : orderId(id), instrument(instrument), newPrice(price), newQuantity(qty) {}
};

// Details for Cancel Order
struct CancelOrderDetails {
    unsigned int orderId;
    std::string instrument;

    CancelOrderDetails(unsigned int id, std::string instrument) : orderId(id), instrument(instrument) {}
};

struct Message {
    MessageType type;
    std::chrono::system_clock::time_point time;
    std::unique_ptr<AddOrderDetails> addOrderDetails;
    std::unique_ptr<ModifyOrderDetails> modifyDetails;
    std::unique_ptr<CancelOrderDetails> cancelDetails;

    // Factory methdos to create diofferent kinds of messages
    static Message createAddOrderMessage(const std::string& instrument, double price, int quantity, bool isBuy, OrderType type) {
        Message msg;
        msg.type = MessageType::ADD_ORDER;
        msg.addOrderDetails = std::make_unique<AddOrderDetails>(instrument, price, quantity, isBuy, type);
        msg.time = currentTimestamp();
        return msg;
    }

    static Message createModifyOrderMessage(unsigned int orderId, std::string instrument, double newPrice, int newQuantity) {
        Message msg;
        msg.type = MessageType::MODIFY_ORDER;
        msg.modifyDetails = std::make_unique<ModifyOrderDetails>(orderId, instrument, newPrice, newQuantity);
        return msg;
    }

    static Message createCancelOrderMessage(unsigned int orderId, std::string instrument) {
        Message msg;
        msg.type = MessageType::CANCEL_ORDER;
        msg.cancelDetails = std::make_unique<CancelOrderDetails>(orderId, instrument);
        return msg;
    }
};

#endif // MESSAGE_HPP
