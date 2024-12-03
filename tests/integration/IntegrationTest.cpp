#include <gtest/gtest.h>
#include "MessageQueue.h"
#include "TCPGateway.h"
#include "MatchingEngine.h"
#include "OrderManager.h"


class IntegrationTest : public ::testing::Test {
protected:

    void SetUp() override 
    {
    }

    void TearDown() override
    {
    }

};

TEST_F(IntegrationTest, FrontSystemAndOrderSystem)
{
    // Use:
    // ./tests --gtest_filter=IntegrationTest.FrontSystemAndOrderSystem
    MessageQueue messageQueue;
    MatchingEngine engine;
    engine.createNewOrderBook("AAPL");
    OrderManager manager{&engine, messageQueue};

    uv_loop_t loop;
    uv_loop_init(&loop);
    auto gateway = std::make_unique<TCPGateway>(&loop, messageQueue);

    manager.start();
    gateway->start("127.0.0.1", 7001);
    std::cout << "Starting event loop. Type 'stop' to terminate." << '\n';

    std::atomic<bool> running(true);
    std::thread inputThread([&running, &gateway, &manager]() {
        std::string input;
        while (running.load()) {
            std::cout << "Enter 'stop' to terminate the services: ";
            std::cin >> input;
            if (input == "stop") {
                running.store(false);
                gateway->stop();
                manager.stop();
                std::cout << "Stopping event loops and manager..." << '\n';
            }
        }
    });


    if (inputThread.joinable()) {
        inputThread.join();
        std::cout << "Event loop and manager stopped" << '\n';
    }

    gateway.reset();

    if (loop.active_handles > 0) {
        uv_stop(&loop);
    }
    uv_loop_close(&loop);
}


