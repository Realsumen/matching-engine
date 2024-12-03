#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include "OrderType.h"
#include "TimestampUtility.h"

// Messages Type
enum class MessageType {
    ADD_ORDER,
    MODIFY_ORDER,
    CANCEL_ORDER,
    UNDEFINED
};

// Details for AddOrder
struct AddOrderDetails {
    std::string instrument;
    double price;
    int quantity;
    bool isBuy;
    OrderType type;

    AddOrderDetails(std::string  instr, const double p, const int qty, const bool buy, const OrderType orderType)
        : instrument(std::move(instr)), price(p), quantity(qty), isBuy(buy), type(orderType) {}
};

// Details for ModifyOrder
struct ModifyOrderDetails {
    unsigned int orderId;
    std::string instrument;
    double newPrice;
    int newQuantity;

    ModifyOrderDetails(const unsigned int id, std::string instrument, const double price, const int qty)
        : orderId(id), instrument(std::move(instrument)), newPrice(price), newQuantity(qty) {}
};

// Details for Cancel Order
struct CancelOrderDetails {
    unsigned int orderId;
    std::string instrument;

    CancelOrderDetails(const unsigned int id, std::string instrument) : orderId(id), instrument(std::move(instrument)) {}
};

struct Message {
    MessageType type = MessageType::UNDEFINED;
    std::chrono::system_clock::time_point time;
    std::unique_ptr<AddOrderDetails> addOrderDetails;
    std::unique_ptr<ModifyOrderDetails> modifyDetails;
    std::unique_ptr<CancelOrderDetails> cancelDetails;

    // Factory methods to create different kinds of messages
    static auto createAddOrderMessage(const std::string& instrument, double price, int quantity, bool isBuy, OrderType type) -> Message {
        Message msg;
        msg.type = MessageType::ADD_ORDER;
        msg.addOrderDetails = std::make_unique<AddOrderDetails>(instrument, price, quantity, isBuy, type);
        msg.time = currentTimestamp();
        return msg;
    }

    static auto createModifyOrderMessage(unsigned int orderId, const std::string& instrument, double newPrice, int newQuantity) -> Message {
        Message msg;
        msg.type = MessageType::MODIFY_ORDER;
        msg.modifyDetails = std::make_unique<ModifyOrderDetails>(orderId, instrument, newPrice, newQuantity);
        return msg;
    }

    static auto createCancelOrderMessage(unsigned int orderId, const std::string& instrument) -> Message {
        Message msg;
        msg.type = MessageType::CANCEL_ORDER;
        msg.cancelDetails = std::make_unique<CancelOrderDetails>(orderId, instrument);
        return msg;
    }
};

#endif // MESSAGE_HPP
