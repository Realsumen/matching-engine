#ifndef ORDER_H
#define ORDER_H

#include <chrono>
#include <string>
#include "utility/include/OrderType.h"

class Order {
public:
    Order(unsigned int id, const std::string &asset, double price, int quantity, bool is_buy, OrderType type);

    void setPrice(double new_price);
    void setQuantity(int new_quantity);
    
    double getPrice() const;          // Getter for price
    int getQuantity() const;          // Getter for quantity
    unsigned int getId() const;       // Getter for id
    std::string getAsset() const;
    bool isBuy() const;               // Getter for is_buy
    OrderType getType() const;        // Getter for type
    std::chrono::time_point<std::chrono::high_resolution_clock> getTimestamp() const; // Getter for timestamp

    void displayOrderInfo() const;

    bool operator==(const Order &other) const;
    bool operator<(const Order &other) const;

    static Order CreateLimitOrder(unsigned int id, const std::string &asset, double price, int quantity, bool is_buy);
    static Order CreateMarketOrder(unsigned int id, const std::string &asset, int quantity, bool is_buy);
    static Order CreateStopOrder(unsigned int id, const std::string &asset, double price, int quantity, bool is_buy);

private:
    unsigned int id;  // order ID
    std::string asset; // asset name
    double price;      // price
    int quantity;      // quantity
    bool is_buy;       // whether it is a buy order
    OrderType type;    // order type
    std::chrono::time_point<std::chrono::high_resolution_clock> timestamp; // timestamp
};

#endif // ORDER_H
