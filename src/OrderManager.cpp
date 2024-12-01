#include <thread>
#include <atomic>
#include <iostream>

#include "MatchingEngine.h"
#include "OrderManager.h"
#include "MessageQueue.h"
#include "IDGenerator.hpp"

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
        if (messageQueue.pop(msg, std::chrono::milliseconds(1000)))
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
    const AddOrderDetails &details = *message.addOrderDetails;
    const unsigned int newID = IDGenerator::getInstance().getNextOrderID();
    const std::string instrument = details.instrument;

    if (!matchingEngine->hasInstrument(instrument))
        throw std::invalid_argument("Unknown Instrument.");

    if (matchingEngine->hasOrderId(newID))
        throw std::invalid_argument("Repeated order ID detected.");

    Order *newOrder = createOrder(details, newID);
    matchingEngine->processNewOrder(newOrder);

}

Order *OrderManager::createOrder(const AddOrderDetails& details, unsigned int orderID) 
{
    switch (details.type)
    {
        case OrderType::MARKET:
            return Order::CreateMarketOrder(orderID, details.instrument, details.quantity, details.isBuy);
        case OrderType::LIMIT:
            return Order::CreateLimitOrder(orderID, details.instrument, details.price, details.quantity, details.isBuy);
        case OrderType::STOP:
            return Order::CreateStopOrder(orderID, details.instrument, details.price, details.quantity, details.isBuy);
        default:
            throw std::invalid_argument("Unknown OrderType.");
    }
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
