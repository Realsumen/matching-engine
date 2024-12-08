// SystemLauncher.h
#ifndef SYSTEM_LAUNCHER_H
#define SYSTEM_LAUNCHER_H

#include <memory>
#include <thread>
#include <atomic>
#include <uv.h>

#include "MessageQueue.h"
#include "MatchingEngine.h"
#include "OrderManager.h"
#include "TCPGateway.h"

struct StopSignal {
    std::shared_ptr<TCPGateway> gateway;
    OrderManager* manager;
    uv_loop_t* loop;
};

class SystemLauncher {
public:
    SystemLauncher(std::string address, int port);

    ~SystemLauncher();

    void run();

    void stop();

    static std::shared_ptr<spdlog::logger> logger;

private:


    static void on_stop_signal(uv_async_t* handle);
    void registerCommands();
    void inputLoop();

    std::string address_;
    int port_;

    uv_loop_t loop_{};
    uv_async_t async_stop_{};
    std::atomic<bool> running_;
    std::thread inputThread_;
    std::unordered_map<std::string, std::function<void()>> commandHandlers;

    std::unique_ptr<MessageQueue> messageQueue_;
    std::unique_ptr<MatchingEngine> engine_;
    std::unique_ptr<OrderManager> manager_;
    std::shared_ptr<TCPGateway> gateway_;

    std::unique_ptr<StopSignal> stopSignal_;
};

#endif // SYSTEM_LAUNCHER_H
