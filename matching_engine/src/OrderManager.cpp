#include <thread>
#include <atomic>
#include <iostream>

#include "MatchingEngine.h"
#include "OrderManager.h"
#include "matching_engine_config.hpp"
#include "MessageQueue.h"
#include "IDGenerator.hpp"
#include "Logger.hpp"

OrderManager::OrderManager(MatchingEngine* engine, MessageQueue& messageQueue, TCPGateway* gateway)
    : matchingEngine(engine), messageQueue(messageQueue), gateway(gateway){}

void OrderManager::start()
{
    if (!managerRunning) {
        managerRunning = true;
        messageProcessingThread = std::thread(&OrderManager::processLoop, this);
    } else {
        std::cout << "OrderManager already running" << '\n';
    }
}

void OrderManager::stop()
{   if (managerRunning) {
        managerRunning = false;
        messageQueue.shutdown();
        if (messageProcessingThread.joinable()) {
            messageProcessingThread.join();
        }
    } else {
        Logger::getLogger(matchingSystemConfig::orderManager::LOGGER_NAME)->error("OrderManager is not running" );
    }
}

void OrderManager::processLoop()
{
    while (managerRunning)
    {
        const auto logger = Logger::getLogger(matchingSystemConfig::orderManager::LOGGER_NAME);
        if (Message msg; messageQueue.pop(msg))
        {
            switch (msg.type)
            {
            case MessageType::ADD_ORDER:
                handleAddMessage(msg);
                logger->info(msg.addOrderDetails->toString());
                break;
            case MessageType::MODIFY_ORDER:
                handleModifyMessage(msg);
                logger->info(msg.modifyDetails->toString());
                break;
            case MessageType::CANCEL_ORDER:
                handleCancelMessage(msg);
                logger->info(msg.cancelDetails->toString());
                break;
            default:
                std::cerr << "Unknown Message Type." << '\n';
                break;
            }
        } else {
            // If messageQueue.pop(), suggesting that the queue is already closed.
            break;
        }
    }
    if (!messageQueue.empty()){
        std::cerr << messageQueue.size() << " messages are in queue not processed.\n";
    }
}

void OrderManager::handleAddMessage(const Message &message)
{
    const AddOrderDetails &details = *message.addOrderDetails;
    const unsigned int newID = IDGenerator::getInstance().getNextOrderID();

    if (const std::string instrument = details.instrument; !matchingEngine->hasInstrument(instrument))
        {
        throw std::invalid_argument("Unknown Instrument.");
        }

    if (matchingEngine->hasOrderId(newID)) {
        throw std::invalid_argument("Repeated order ID detected.");
    }

    Order *newOrder = createOrder(details, newID);
    matchingEngine->processNewOrder(newOrder);

    const std::string response = "Order added successfully with ID: " + std::to_string(newID);

    if (gateway != nullptr) {
        gateway->queueMessageToSend(message.client_id, response);
    }

}

auto OrderManager::isRunning() -> bool {
    return managerRunning;
}

auto OrderManager::createOrder(const AddOrderDetails& details, unsigned int orderID) -> Order * 
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
