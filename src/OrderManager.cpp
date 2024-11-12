#include <thread>
#include <atomic>
#include <iostream>

#include "Order.h"
#include "OrderBook.h"
#include "MatchingEngine.h"
#include "OrderManager.h"
#include "../utility/include/MessageQueue.h"
#include "../utility/include/Message.hpp"
#include "../utility/include/IDGenerator.hpp"

OrderManager::OrderManager(MatchingEngine* engine, MessageQueue& queue)
    : matchingEngine(engine), messageQueue(queue){}

void OrderManager::start()
{
    managerRunning = true;
    messageProcessingThread = std::thread(&OrderManager::processLoop, this);
}

void OrderManager::stop()
{
    managerRunning = false;
    if (messageProcessingThread.joinable())
    {
        messageProcessingThread.join();
    }
}

void OrderManager::processLoop()
{
    while (managerRunning)
    {
        Message msg;
        if (messageQueue.pop(msg))
        {
            switch (msg.type)
            {
            case MessageType::ADD_ORDER:
                handleAddMessage(msg);
                break;
            case MessageType::MODIFY_ORDER:
                handleModifyMessage(msg);
                break;
            case MessageType::CANCEL_ORDER:
                handleCancelMessage(msg);
                break;
            default:
                std::cerr << "Unknown Message Type." << std::endl;
                break;
            }
        }
        
    }
}

void OrderManager::handleAddMessage(const Message &message)
{
    // Add new order according to the OrderType in of the message
    const AddOrderDetails &details = *message.addOrderDetails;
    unsigned int newID = IDGenerator::getInstance().getNextOrderID();
    Order *newOrder = nullptr;
    
    switch (details.type)
    {
        case OrderType::MARKET:
            newOrder = Order::CreateMarketOrder(newID, details.instrument, details.quantity, details.isBuy);
            break;
        case OrderType::LIMIT:
            newOrder = Order::CreateLimitOrder(newID, details.instrument, details.price, details.quantity, details.isBuy);
            break;
        case OrderType::STOP:
            newOrder = Order::CreateStopOrder(newID, details.instrument, details.price, details.quantity, details.isBuy);
            break;
        default:
            break;
    }

    matchingEngine->processNewOrder(newOrder);
}

void OrderManager::handleModifyMessage(const Message &message)
{
    const ModifyOrderDetails &details = *message.modifyDetails;
    matchingEngine->modifyOrder(details.orderId, details.instrument, details.newPrice, details.newQuantity);
}

void OrderManager::handleCancelMessage(const Message &message)
{
    const CancelOrderDetails &details = *message.cancelDetails;
    matchingEngine->cancelOrder(details.orderId, details.instrument);
}
