#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "MessageQueue.h"
#include "Message.hpp"

class MessageQueueTest : public ::testing::Test {
protected:
    MessageQueue queue;

    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(MessageQueueTest, IsEmptyInitially) {
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}


TEST_F(MessageQueueTest, PushAndPopSingleMessage) {
    Message msg = Message::createAddOrderMessage("AAPL", 155.0, 100, true, OrderType::LIMIT);
    queue.push(std::move(msg));
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.size(), 1);

    Message poppedMsg;
    bool success = queue.pop(poppedMsg);
    EXPECT_TRUE(success);
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

TEST_F(MessageQueueTest, TryPopSuccess) {
    Message msg = Message::createAddOrderMessage("AAPL", 155.0, 100, true, OrderType::LIMIT);
    queue.push(std::move(msg));

    Message poppedMsg;
    bool success = queue.tryPop(poppedMsg);
    EXPECT_TRUE(success);
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(poppedMsg.time, msg.time);
}

TEST_F(MessageQueueTest, TryPopFailureWhenEmpty) {
    Message poppedMsg;
    bool success = queue.tryPop(poppedMsg);
    EXPECT_FALSE(success);
}

TEST_F(MessageQueueTest, QueueSize) {
    EXPECT_EQ(queue.size(), 0);
    Message msg1 = Message::createAddOrderMessage("AAPL", 155.0, 100, true, OrderType::LIMIT);
    Message msg2 = Message::createAddOrderMessage("AAPL", 170.0, 150, false, OrderType::LIMIT);
    queue.push(std::move(msg1));
    EXPECT_EQ(queue.size(), 1);
    queue.push(std::move(msg2));
    EXPECT_EQ(queue.size(), 2);

    Message poppedMsg;
    queue.pop(poppedMsg);
    EXPECT_EQ(queue.size(), 1);
    queue.pop(poppedMsg);
    EXPECT_EQ(queue.size(), 0);
}

TEST_F(MessageQueueTest, MultipleProducersSingleConsumer) {
    const int numProducers = 10;
    const int messagesPerProducer = 100;
    std::vector<std::thread> producers;
    std::thread consumer;
    std::mutex consumedMutex;
    std::vector<Message> consumedMessages;

    for (int i = 0; i < numProducers; ++i) {
        producers.emplace_back([&, i]() {
            for (int j = 0; j < messagesPerProducer; ++j) {
                Message msg = Message::createAddOrderMessage("AAPL", 155.0 + i, 100 + j, true, OrderType::LIMIT);
                queue.push(std::move(msg));
            }
        });
    }

    consumer = std::thread([&]() {
        while (true) {
            Message msg;
            // Consumer get message from the queue
            while (!queue.pop(msg)) {
                std::this_thread::yield(); // If the queue is empty, the consumer will release the thread
            }

            {
                std::lock_guard<std::mutex> lock(consumedMutex);
                consumedMessages.push_back(std::move(msg));
                if (consumedMessages.size() == numProducers * messagesPerProducer) {
                    break; // Break if all the messages were consumed
                }
            }
        }
    });

    for (auto& producer : producers) {
        producer.join();
    }

    consumer.join();
    
    std::cout << consumedMessages.size() << std::endl;
    EXPECT_EQ(consumedMessages.size(), numProducers * messagesPerProducer);
    EXPECT_TRUE(queue.empty());
}
