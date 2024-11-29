#include <gtest/gtest.h>
#include <iostream>
#include <vector>
#include <thread>
#include "MessageQueue.h"
#include "Message.hpp"
#include "MatchingEngine.h"
#include "OrderManager.h"

class OrderManagerTest : public ::testing::Test {
protected:
    MessageQueue queue;
    MatchingEngine engine;
    OrderManager manager = OrderManager(&engine, queue);

    void SetUp() override 
    {
        engine.createNewOrderBook("AAPL");
    }

    void TearDown() override {}
};

TEST_F(OrderManagerTest, BasicTest)
{

    Message msg = Message::createAddOrderMessage("AAPL", 155.0, 100, false, OrderType::LIMIT);
    manager.handleAddMessage(msg);

    engine.getOrderBookForRead("AAPL")->printOrderBook();
    ASSERT_NE(nullptr, engine.getOrderBookForRead("AAPL")->getBestAsk());
    EXPECT_EQ(155, engine.getOrderBookForRead("AAPL")->getBestAsk()->getPrice());
}

TEST_F(OrderManagerTest, MultithreadMessageQueueTest)
{
    const int numProducers = 2;
    const int messagesPerProducer = 1;
    std::vector<std::thread> producers;
    std::thread consumer;
    std::mutex consumedMutex;
    std::vector<Message> consumedMessages;

    for (int i = 0; i < numProducers; ++i) {
        producers.emplace_back([&, i]() {
            for (int j = 0; j < messagesPerProducer; ++j) {
                Message msg = Message::createAddOrderMessage("AAPL", 160.0 + i, 200 + j, false, OrderType::LIMIT);
                std::cout << "Pushing message to queue..." << std::endl;
                queue.push(std::move(msg));
            }
        });
    }

    manager.start();
    for (auto& producer : producers) {
        producer.join();
    }
    manager.stop();

    engine.getOrderBookForRead("AAPL")->printOrderBook();
    ASSERT_NE(nullptr, engine.getOrderBookForRead("AAPL")->getBestAsk());
    EXPECT_EQ(160, engine.getOrderBookForRead("AAPL")->getBestAsk()->getPrice());
}