#include <iostream>
#include "TimestampUtility.h"
#include "OrderType.h"
#include "Order.h"

Order::Order(const unsigned int id, std::string asset, const double price, const int quantity, const bool is_buy, const OrderType type)
    : id(id), asset(std::move(asset)), price(price), quantity(quantity), is_buy(is_buy), type(type),
      timestamp(currentTimestamp())
{
    if (price > 0.0)
    {
        this->price = price;
    }
    else if (type != OrderType::MARKET)
    {
        throw std::invalid_argument("Price must be greater than zero.");
    }

    if (quantity > 0)
    {
        this->quantity = quantity;
    }
    else
    {
        throw std::invalid_argument("Quantity must be greater than zero.");
    }
}

void Order::setPrice(const double new_price)
{
    if (new_price > 0.0)
    {
        price = new_price;
    }
    else
    {
        throw std::invalid_argument("Price cannot be negative.");
    }
}

void Order::setQuantity(const int new_quantity)
{
    if (new_quantity < 0)
    {
        throw std::invalid_argument("Quantity must be greater than or equal to zero.");
    }
    if (new_quantity == 0 && getType() != OrderType::MARKET)
    {
        throw std::invalid_argument("Quantity of Limit Order and Stop Order must be greater than zero.");
    }
    quantity = new_quantity;
}

auto Order::getPrice() const -> double
{
    return price;
}

auto Order::getQuantity() const -> int
{
    return quantity;
}

unsigned int Order::getId() const
{
    return id;
}

std::string Order::getAsset() const
{
    return asset;
}

bool Order::isBuy() const
{
    return is_buy;
}

OrderType Order::getType() const
{
    return type;
}

auto Order::getTimestamp() const -> std::chrono::system_clock::time_point
{
    return timestamp;
}

void Order::displayOrderInfo() const
{
    std::cout << "Order ID: " << id << "\n";
    std::cout << "Asset: " << asset << "\n";
    std::cout << "Price: " << price << "\n";
    std::cout << "Quantity: " << quantity << "\n";
    std::cout << "Type: " << (type == OrderType::LIMIT ? "LIMIT" : type == OrderType::MARKET ? "MARKET"
                                                                                             : "STOP")
              << "\n";
    std::cout << "Direction: " << (is_buy ? "Buy" : "Sell") << "\n";
    std::cout << "Timestamp: " << timestampToString(timestamp)
              << " \n";
}

auto Order::operator==(const Order &other) const -> bool
{
    /*
        This method only compare the price between two orders.
    */
    bool flag = (this->price == other.price);
    return flag;
}

auto Order::operator<(const Order &other) const -> bool
{
    /*
        This method only compare the price between two orders.
    */
    bool flag = (this->price < other.price);
    return flag;
}

Order* Order::CreateMarketOrder(const unsigned int id, const std::string &asset, const int quantity, const bool is_buy)
{
    return new Order(id, asset, -1.0, quantity, is_buy, OrderType::MARKET);
}

Order* Order::CreateLimitOrder(const unsigned int id, const std::string &asset, const double price, const int quantity, const bool is_buy)
{
    return new Order(id, asset, price, quantity, is_buy, OrderType::LIMIT);
}

Order* Order::CreateStopOrder(const unsigned int id, const std::string &asset, const double price, const int quantity, const bool is_buy)
{
    return new Order(id, asset, price, quantity, is_buy, OrderType::STOP);
}