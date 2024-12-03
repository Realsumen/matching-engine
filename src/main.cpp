#include <vector>
#include <thread>
#include "MessageQueue.h"
#include "Message.hpp"
#include "MatchingEngine.h"
#include "OrderManager.h"
#include <iostream>
#include "Order.h"
#include "Trade.h"
#include <uv.h>
#include <iostream>


int main()
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

