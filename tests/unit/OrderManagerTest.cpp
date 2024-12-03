#include <gtest/gtest.h>
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
    OrderManager manager{&engine, queue}; // 直接初始化
    // OrderManager manager = OrderManager(&engine, queue);

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

    manager.handleAddMessage(msg);
}

// TEST_F(OrderManagerTest, MultithreadMessageQueueTest)
// {
//     const int numProducers = 3;
//     const int messagesPerProducer = 2;
//     std::vector<std::thread> producers;
//     std::vector<Message> consumedMessages;
//
//     producers.reserve(numProducers);
//     for (int i = 0; i < numProducers; ++i) {
//             producers.emplace_back([&, i]() {
//             for (int j = 0; j < messagesPerProducer; ++j) {
//                 Message msg = Message::createAddOrderMessage("AAPL", 160.0 + i, 200 + j, false, OrderType::LIMIT);
//                 queue.push(std::move(msg));
//             }
//         });
//     }
//
//     manager.start();
//
//
//     std::atomic<bool> running(true);
//     std::thread inputThread([this, &running]() {
//         while (running.load()) {
//             std::this_thread::sleep_for(std::chrono::milliseconds(100));
//             if (!manager.isRunning()) {
//                 std::cout << "Stopping event loop..." << '\n';
//                 running.store(false);
//             }
//
//             running.store(false);
//             std::cout << "Stopping event loop..." << '\n';
//
//         }
//     });
//
//     for (auto& producer : producers) {
//         producer.join();
//     }
//
//     manager.stop();
//
//     inputThread.join();
// }

TEST_F(OrderManagerTest, MultithreadMessageQueueTest1)
{
    MessageQueue queue;
    MatchingEngine engine;
    engine.createNewOrderBook("AAPL");

    const int numProducers = 3;
    const int messagesPerProducer = 2;
    OrderManager manager{&engine, queue};
    std::vector<std::thread> producers;
    std::vector<Message> consumedMessages;

    producers.reserve(numProducers);
    for (int i = 0; i < numProducers; ++i) {
        producers.emplace_back([&, i]() {
            for (int j = 0; j < messagesPerProducer; ++j) {
                Message msg = Message::createAddOrderMessage("AAPL", 160.0 + i, 200 + j, false, OrderType::LIMIT);
                queue.push(std::move(msg));
            }
        });
    }

    for (auto& producer : producers) {
        producer.join();
    }
    manager.start();

    std::atomic<bool> done(true);
    std::thread inputThread([&]() {
        std::string command;
        while (done.load()) {
            std::cout << "Enter 'stop' to terminate the service: ";
            std::cin >> command;
            if (command == "stop") {
                done.store(false);
                manager.stop();
                std::cout << "Stopping event loop..." << '\n';
            }
        }
    });
    if (inputThread.joinable()) {
        inputThread.join();
    }
}
