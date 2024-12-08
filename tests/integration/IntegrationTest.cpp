#include <gtest/gtest.h>
#include "MessageQueue.h"
#include "TCPGateway.h"
#include "MatchingEngine.h"
#include "OrderManager.h"
#include "test_config.hpp"
#include <mutex>
#include <memory>

struct StopSignal {
    std::shared_ptr<TCPGateway> gateway;
    std::shared_ptr<OrderManager> manager;
    uv_loop_t* loop;
};

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

void on_stop_signal(uv_async_t* handle) {
    auto* signal = static_cast<StopSignal*>(handle->data);

    // 停止 gateway 和 manager
    if (signal->gateway) {
        signal->gateway->stop();
    }
    if (signal->manager) {
        signal->manager->stop();
    }

    // 停止事件循环
    uv_stop(signal->loop);

    // 关闭 async 句柄，但不删除 signal
    uv_close(reinterpret_cast<uv_handle_t*>(handle), nullptr);
}

TEST_F(IntegrationTest, FrontSystemAndOrderSystem) {
    // ./tests --gtest_filter=IntegrationTest.FrontSystemAndOrderSystem
    MessageQueue messageQueue;
    MatchingEngine engine;
    engine.createNewOrderBook(TEST_Config::Integration::INSTRUMENT);

    // 使用 shared_ptr 管理生命周期
    auto manager = std::make_shared<OrderManager>(&engine, messageQueue);
    uv_loop_t loop;
    uv_loop_init(&loop);
    auto gateway = std::make_shared<TCPGateway>(&loop, messageQueue);

    // 创建 uv_async_t 句柄
    uv_async_t async_stop;
    async_stop.data = nullptr; // 初始化为空

    // 启动 manager 和 gateway
    manager->start();
    gateway->start(TEST_Config::Integration::ADDRESS, TEST_Config::Integration::PORT);
    std::cout << "Starting event loop. Type 'stop' to terminate." << '\n';

    // 初始化 StopSignal
    StopSignal stopSignal{gateway, manager, &loop};
    async_stop.data = &stopSignal;
    uv_async_init(&loop, &async_stop, on_stop_signal);

    std::mutex cout_mutex;
    std::atomic<bool> running(true);

    // 输入线程，等待用户输入“stop”
    std::thread inputThread([&running, &async_stop, &cout_mutex]() {
        std::string input;
        while (running.load()) {
            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "Enter 'stop' to terminate the services: ";
            }
            std::cin >> input;
            if (input == "stop") {
                running.store(false);
                uv_async_send(&async_stop);
                {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    std::cout << "Sent stop signal to event loop..." << '\n';
                }
            }
        }
    });

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "UV loop is starting..." << '\n';
    }
    uv_run(&loop, UV_RUN_DEFAULT);

    if (inputThread.joinable()) {
        inputThread.join();
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Event loop and manager stopped" << '\n';
        }
    }

    // 确保 loop 在句柄关闭后才关闭
    uv_loop_close(&loop);
}
