#include <iostream>
#include "TimestampUtility.h"
#include "OrderType.h"
#include "Order.h"

Order::Order(unsigned int id, const std::string &asset, double price, int quantity, bool is_buy, OrderType type)
    : id(id), asset(asset), price(price), quantity(quantity), is_buy(is_buy), type(type),
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

void Order::setPrice(double new_price)
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

void Order::setQuantity(int new_quantity)
{
    if (new_quantity > 0)
    {
        quantity = new_quantity;
    }
    else
    {
        throw std::invalid_argument("Quantity must be greater than zero.");
    }
}

double Order::getPrice() const
{
    return price;
}

int Order::getQuantity() const
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

std::chrono::system_clock::time_point Order::getTimestamp() const
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

bool Order::operator==(const Order &other) const
{
    /*
        This method only compare the price between two orders.
    */
    bool flag = (this->price == other.price);
    return flag;
}

bool Order::operator<(const Order &other) const
{
    /*
        This method only compare the price between two orders.
    */
    bool flag = (this->price < other.price);
    return flag;
}

Order* Order::CreateMarketOrder(unsigned int id, const std::string &asset, int quantity, bool is_buy)
{
    return new Order(id, asset, -1.0, quantity, is_buy, OrderType::MARKET);
}

Order* Order::CreateLimitOrder(unsigned int id, const std::string &asset, double price, int quantity, bool is_buy)
{
    return new Order(id, asset, price, quantity, is_buy, OrderType::LIMIT);
}

Order* Order::CreateStopOrder(unsigned int id, const std::string &asset, double price, int quantity, bool is_buy)
{
    return new Order(id, asset, price, quantity, is_buy, OrderType::STOP);
}