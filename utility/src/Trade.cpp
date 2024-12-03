#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "Trade.h"
#include "utility_config.hpp"
#include "TimestampUtility.h"

Trade::Trade(const unsigned int trade_id, const unsigned int buy_order_id, const unsigned int sell_order_id, std::string  asset, const double price, const int quantity) : trade_id(trade_id), buy_order_id(buy_order_id), sell_order_id(sell_order_id),
                                                                                                                       asset(std::move(asset)), price(price), quantity(quantity), timestamp(currentTimestamp()),
                                                                                                                       buyOrderStatus(TradeStatus::UNDEFINED), sellOrderStatus(TradeStatus::UNDEFINED) {}

auto Trade::getAsset() const -> const std::string &
{
    return asset;
}

auto Trade::getTradeId() const -> unsigned int
{
    return trade_id;
}

auto Trade::getBuyOrderId() const -> unsigned int
{
    return buy_order_id;
}

auto Trade::getSellOrderId() const -> unsigned int
{
    return sell_order_id;
}

auto Trade::getPrice() const -> double
{
    return price;
}

auto Trade::getTradeValue() const -> double
{
    return price * quantity;
}

auto Trade::getQuantity() const -> int
{
    return quantity;
}

auto Trade::getBuyOrderStatus() const -> TradeStatus
{
    return buyOrderStatus;
}

auto Trade::getSellOrderStatus() const -> TradeStatus
{
    return sellOrderStatus;
}

void Trade::setBuyOrderStatus(TradeStatus status)
{
    this->buyOrderStatus = status;
}

void Trade::setSellOrderStatus(TradeStatus status)
{
    this->sellOrderStatus = status;
}

auto Trade::getTimestamp() const -> std::chrono::system_clock::time_point
{
    return timestamp;
}

auto Trade::toString(const std::string& format) const -> std::string {
    std::ostringstream oss;

    if (format == "default") {
        auto appendField = [&oss](const std::string& label, const auto& value, bool format = false) {
            oss << std::setw(Utility_Config::Trade::DEFAULT_ALIGNMENT_DISPLAY) << label;
            if (format) 
            {
                oss << std::fixed << std::setprecision(Utility_Config::Trade::DEFAULT_PRECISION_DISPLAY) << value;
            } else {
                oss << value;
            }
        };

        appendField("Trade ID: ", trade_id);
        appendField("Buy Order ID: ", buy_order_id);
        appendField("Sell Order ID: ", sell_order_id);
        appendField("Asset: ", asset);
        appendField("Price: ", price, true);
        appendField("Quantity: ", quantity);
        appendField("Trade Value: ", getTradeValue(), true);
        appendField("Buy Order Status: ", static_cast<int>(buyOrderStatus));
        appendField("Sell Order Status: ", static_cast<int>(sellOrderStatus));
        appendField("Timestamp: ", timestampToString(timestamp));
    } else if (format == "json") {
        oss << "{"
            << "\"TradeID\":" << trade_id << ","
            << "\"BuyOrderID\":" << buy_order_id << ","
            << "\"SellOrderID\":" << sell_order_id << ","
            << R"("Asset":")" << asset << "\","
            << "\"Price\":" << std::fixed << std::setprecision(Utility_Config::Trade::DEFAULT_PRECISION_DISPLAY) << price << ","
            << "\"Quantity\":" << quantity << ","
            << "\"TradeValue\":" << std::fixed << std::setprecision(Utility_Config::Trade::DEFAULT_PRECISION_DISPLAY) << getTradeValue() << ","
            << "\"BuyOrderStatus\":" << static_cast<int>(buyOrderStatus) << ","
            << "\"SellOrderStatus\":" << static_cast<int>(sellOrderStatus) << ","
            << R"("Timestamp":")" << timestampToString(timestamp) << "\""
            << "}";
    } else if (format == "csv") {
        oss << trade_id << ","
            << buy_order_id << ","
            << sell_order_id << ","
            << asset << ","
            << std::fixed << std::setprecision(Utility_Config::Trade::DEFAULT_PRECISION_DISPLAY) << price << ","
            << quantity << ","
            << std::fixed << std::setprecision(Utility_Config::Trade::DEFAULT_PRECISION_DISPLAY) << getTradeValue() << ","
            << static_cast<int>(buyOrderStatus) << ","
            << static_cast<int>(sellOrderStatus) << ","
            << timestampToString(timestamp);
    } else {
        throw std::invalid_argument("Unsupported format: " + format);
    }

    return oss.str();
}
