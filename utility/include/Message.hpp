#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <iomanip>
#include <string>
#include <sstream>

#include "OrderType.h"
#include "TimestampUtility.h"

// Messages Type
enum class MessageType : std::uint8_t {
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

    [[nodiscard]] auto toString(const std::string& format = "default") const -> std::string;
};

// Details for ModifyOrder
struct ModifyOrderDetails {
    unsigned int orderId;
    std::string instrument;
    double newPrice;
    int newQuantity;

    ModifyOrderDetails(const unsigned int id, std::string instrument, const double price, const int qty)
        : orderId(id), instrument(std::move(instrument)), newPrice(price), newQuantity(qty) {}
    [[nodiscard]] auto toString(const std::string& format = "default") const -> std::string;

};

// Details for Cancel Order
struct CancelOrderDetails {
    unsigned int orderId;
    std::string instrument;

    CancelOrderDetails(const unsigned int id, std::string instrument) : orderId(id), instrument(std::move(instrument)) {}
    [[nodiscard]] auto toString(const std::string& format = "default") const -> std::string;

};

struct Message {
    MessageType type = MessageType::UNDEFINED;
    unsigned int client_id{};
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

inline auto AddOrderDetails::toString(const std::string& format) const -> std::string {
    std::ostringstream oss;

    if (format == "default") {
        oss << "Instrument: " << instrument << ", "
            << "Price: " << std::fixed << std::setprecision(2) << price << ", "
            << "Quantity: " << quantity << ", "
            << "IsBuy: " << (isBuy ? "Buy" : "Sell") << ", "
            << "OrderType: " << static_cast<int>(type);
    } else if (format == "json") {
        oss << "{"
            << R"("Instrument":")" << instrument << R"(",)"
            << R"("Price":)" << std::fixed << std::setprecision(2) << price << ","
            << R"("Quantity":)" << quantity << ","
            << R"("IsBuy":)" << (isBuy ? "true" : "false") << ","
            << R"("OrderType":)" << static_cast<int>(type)
            << "}";
    } else if (format == "csv") {
        oss << instrument << ","
            << std::fixed << std::setprecision(2) << price << ","
            << quantity << ","
            << (isBuy ? "Buy" : "Sell") << ","
            << static_cast<int>(type);
    } else {
        throw std::invalid_argument("Unsupported format: " + format);
    }

    return oss.str();
}

inline auto ModifyOrderDetails::toString(const std::string& format) const -> std::string {
    std::ostringstream oss;

    if (format == "default") {
        oss << "OrderID: " << orderId << ", "
            << "Instrument: " << instrument << ", "
            << "NewPrice: " << std::fixed << std::setprecision(2) << newPrice << ", "
            << "NewQuantity: " << newQuantity;
    } else if (format == "json") {
        oss << "{"
            << R"("OrderID":)" << orderId << ","
            << R"("Instrument":")" << instrument << R"(",)"
            << R"("NewPrice":)" << std::fixed << std::setprecision(2) << newPrice << ","
            << R"("NewQuantity":)" << newQuantity
            << "}";
    } else if (format == "csv") {
        oss << orderId << ","
            << instrument << ","
            << std::fixed << std::setprecision(2) << newPrice << ","
            << newQuantity;
    } else {
        throw std::invalid_argument("Unsupported format: " + format);
    }

    return oss.str();
}

inline auto CancelOrderDetails::toString(const std::string& format) const -> std::string {
    std::ostringstream oss;

    if (format == "default") {
        oss << "OrderID: " << orderId << ", "
            << "Instrument: " << instrument;
    } else if (format == "json") {
        oss << "{"
            << R"("OrderID":)" << orderId << ","
            << R"("Instrument":")" << instrument << R"(")"
            << "}";
    } else if (format == "csv") {
        oss << orderId << ","
            << instrument;
    } else {
        throw std::invalid_argument("Unsupported format: " + format);
    }

    return oss.str();
}




#endif // MESSAGE_HPP
