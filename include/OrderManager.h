#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H

#include "Order.h"
#include "OrderBook.h"
#include "MatchingEngine.h"
#include "../utility/include/IDGenerator.hpp"

enum class MessageType {
    NEW_ORDER,
    MODIFY_ORDER,
    CANCEL_ORDER
};

struct OrderMessage {
    MessageType type;
    std::shared_ptr<Order> order; // 对于新增订单
    unsigned int orderId;         // 对于修改和取消订单
    double newPrice;              // 对于修改订单
    int newQuantity;              // 对于修改订单
};

class OrderManager
{
    // Constructor
    OrderManager(MatchingEngine* engine);

    // Process New Order Message
    void handleNewOrder(Order* order);

    // Process modification message

    // Process cancel message
    
private:
    MatchingEngine* matchingEngine;

};

#endif // ORDERMANAGER_H
