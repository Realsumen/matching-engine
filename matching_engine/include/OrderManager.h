#ifndef ORDER_MANAGER_H
#define ORDER_MANAGER_H

#include <atomic>
#include <thread>
#include "MatchingEngine.h"
#include "Message.hpp"
#include "MessageQueue.h"
#include "Order.h"
#include "OrderBook.h"

class OrderManager
{
public:
    OrderManager(MatchingEngine* engine, MessageQueue& messageQueue);

    OrderManager(const OrderManager&) = delete;
    auto operator=(const OrderManager&) -> OrderManager& = delete;

    void start();

    void stop();

    void handleAddMessage(const Message& message);

    auto isRunning() -> bool;

private:
    MatchingEngine* matchingEngine; // Matching Engine pointer
    MessageQueue& messageQueue;     // Reference to message queue

    std::thread messageProcessingThread;      // Thread for processing messages
    std::atomic<bool> managerRunning{false};         // Flag to control message processing loop

    // Messages processing loop
    void processLoop();

    void handleModifyMessage(const Message& message);

    void handleCancelMessage(const Message& message);

    auto createOrder(const AddOrderDetails& details, unsigned int orderID) -> Order* ;

};

#endif // ORDER_MANAGER_H
