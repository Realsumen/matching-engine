#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "Trade.h"
#include "TimestampUtility.h"

Trade::Trade(int trade_id, int buy_order_id, int sell_order_id, const std::string asset, double price, int quantity) : trade_id(trade_id), buy_order_id(buy_order_id), sell_order_id(sell_order_id),
                                                                                                                       asset(asset), price(price), quantity(quantity), timestamp(currentTimestamp()),
                                                                                                                       buyOrderStatus(TradeStatus::UNDEFINED), sellOrderStatus(TradeStatus::UNDEFINED) {}

const std::string &Trade::getAsset() const
{
    return asset;
}

unsigned int Trade::getTradeId() const
{
    return trade_id;
}

unsigned int Trade::getBuyOrderId() const
{
    return buy_order_id;
}

unsigned int Trade::getSellOrderId() const
{
    return sell_order_id;
}

double Trade::getPrice() const
{
    return price;
}

double Trade::getTradeValue() const
{
    return price * quantity;
}

int Trade::getQuantity() const
{
    return quantity;
}

TradeStatus Trade::getBuyOrderStatus() const
{
    return buyOrderStatus;
}

TradeStatus Trade::getSellOrderStatus() const
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

std::chrono::system_clock::time_point Trade::getTimestamp() const
{
    return timestamp;
}

std::string Trade::toString() const
{
    std::ostringstream oss;

    // Left aligned
    oss << std::left;
    oss << std::setw(20) << "Trade ID:" << trade_id << '\n'
        << std::setw(20) << "Buy Order ID:" << buy_order_id << '\n'
        << std::setw(20) << "Sell Order ID:" << sell_order_id << '\n'
        << std::setw(20) << "Asset:" << asset << '\n'
        << std::setw(20) << "Price:" << std::fixed << std::setprecision(2) << price << '\n'
        << std::setw(20) << "Quantity:" << quantity << '\n'
        << std::setw(20) << "Trade Value:" << std::fixed << std::setprecision(2) << getTradeValue() << '\n'
        << std::setw(20) << "Buy Order Status:" << static_cast<int>(buyOrderStatus) << '\n'
        << std::setw(20) << "Sell Order Status:" << static_cast<int>(sellOrderStatus) << '\n'
        << std::setw(20) << "Timestamp:" << timestampToString(timestamp);

    return oss.str();
}
