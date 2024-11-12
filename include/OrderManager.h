#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H

#include <thread>
#include <atomic>
#include "Order.h"
#include "OrderBook.h"
#include "MatchingEngine.h"
#include "../utility/include/MessageQueue.h"
#include "../utility/include/Message.hpp"
#include "../utility/include/IDGenerator.hpp"

class OrderManager
{
    // Constructor
    OrderManager(MatchingEngine* engine, MessageQueue& messageQueue);

    // Begin processing messages from the messageQueue
    void start();

    // Stop Processing Orders from the messageQueue
    void stop();

private:
    MatchingEngine* matchingEngine; // Matching Engine pointer
    MessageQueue& messageQueue;     // Reference to message queue

    std::thread messageProcessingThread;      // Thread for processing messages
    std::atomic<bool> managerRunning{false};         // Flag to control message processing loop

    // Messages processing loop
    void processLoop();

    void handleAddMessage(const Message& message);

    void handleModifyMessage(const Message& message);

    void handleCancelMessage(const Message& message);

};

#endif // ORDERMANAGER_H
