#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H

#include <thread>
#include <atomic>
#include "Order.h"
#include "OrderBook.h"
#include "MatchingEngine.h"
#include "MessageQueue.h"
#include "Message.hpp"
#include "IDGenerator.hpp"

class OrderManager
{
public:
    OrderManager(MatchingEngine* engine, MessageQueue& messageQueue);

    void start();

    void stop();

    void handleAddMessage(const Message& message);

private:
    MatchingEngine* matchingEngine; // Matching Engine pointer
    MessageQueue& messageQueue;     // Reference to message queue

    std::thread messageProcessingThread;      // Thread for processing messages
    std::atomic<bool> managerRunning{false};         // Flag to control message processing loop

    // Messages processing loop
    void processLoop();

    void handleModifyMessage(const Message& message);

    void handleCancelMessage(const Message& message);

    Order* createOrder(const AddOrderDetails& details, unsigned int orderID) ;

};

#endif // ORDERMANAGER_H
