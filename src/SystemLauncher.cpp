#include "SystemLauncher.h"
#include <iostream>
#include "Logger.hpp"
#include "config.hpp"

using namespace systemLauncher;

std::shared_ptr<spdlog::logger> SystemLauncher::logger = nullptr;

SystemLauncher::SystemLauncher(std::string address, const int port)
    : address_(std::move(address)), port_(port), async_stop_(), running_(false), stopSignal_(nullptr)
{
    logger = Logger::getLogger(LOGGER_NAME, true);
    logger->set_pattern(LOGGER_PATTERN); // 设置仅显示日志消息
    registerCommands();
    uv_loop_init(&loop_);
}

SystemLauncher::~SystemLauncher()
{
    if (running_) {
        stop();
    }
    while (uv_loop_alive(&loop_) != 0) {
        uv_run(&loop_, UV_RUN_ONCE);
    }

    uv_loop_close(&loop_);
}

void SystemLauncher::run()
{
    messageQueue_ = std::make_unique<MessageQueue>();
    engine_ = std::make_unique<MatchingEngine>();
    manager_ = std::make_unique<OrderManager>(engine_.get(), *messageQueue_);

    gateway_ = std::make_shared<TCPGateway>(&loop_, *messageQueue_);
    engine_->createNewOrderBook(DEFAULT_ORDERBOOK_INSTRUMENT);

    manager_->start();
    gateway_->start(address_, port_);
    logger->info(LOG_EVENT_LOOP_STARTED);


    if (const int ret = uv_async_init(&loop_, &async_stop_, on_stop_signal); ret != 0) {
        logger->error(LOG_INIT_ASYNC_FAILED, uv_strerror(ret));
        return;
    }

    stopSignal_ = std::make_unique<StopSignal>();
    stopSignal_->gateway = gateway_;
    stopSignal_->manager = manager_.get();
    stopSignal_->loop = &loop_;
    async_stop_.data = stopSignal_.get();

    // System Input Thread
    running_ = true;
    inputThread_ = std::thread(&SystemLauncher::inputLoop, this);

    logger->info(LOG_INPUT_EVENT_LOOP_STARTED);

    const int run_ret = uv_run(&loop_, UV_RUN_DEFAULT);
    if (run_ret < 0) {
        logger->error(LOG_EVENT_LOOP_ERROR);
        stopSignal_ = nullptr;
    }

    if (inputThread_.joinable()) {
        inputThread_.join();
        logger->info(LOG_EVENT_LOOP_STOPPED);
    }
}


void SystemLauncher::on_stop_signal(uv_async_t* handle)
{
    const auto* signal = static_cast<StopSignal*>(handle->data);
    logger->info(LOG_STOP_SIGNAL_RECEIVED);

    if (signal->gateway) {
        signal->gateway->stop();
    }
    if (signal->manager != nullptr) {
        signal->manager->stop();
    }

    uv_stop(signal->loop);

    uv_close(reinterpret_cast<uv_handle_t*>(handle),
        [](uv_handle_t* handle) {
            logger->info(LOG_ASYNC_STOP_CLOSED);
        }
    );
}


void SystemLauncher::stop()
{
    if (running_) {
        running_ = false;
        if (uv_is_closing(reinterpret_cast<uv_handle_t*>(&async_stop_)) == 0) {
            uv_async_send(&async_stop_);
        }
    }
}

void SystemLauncher::registerCommands() {

    commandHandlers["stop"] = [this]() {
        logger->info(LOG_COMMAND_RECEIVED, STOP_COMMAND);
        stop();
    };
    commandHandlers["help"] = [this]() {
        logger->info(LOG_COMMAND_RECEIVED, HELP_COMMAND);
        logger->info(CMD_HELP_MESSAGE);
    };

    commandHandlers["create_orderbook"] = [this]() {
        std::string newInstrument;
        logger->info(CMD_ENTER_INSTRUMENT);
        std::getline(std::cin, newInstrument);
        if (newInstrument.empty()) {
            logger->warn(CMD_EMPTY_INSTRUMENT);
            return;
        }

        if (bool success = engine_->createNewOrderBook(newInstrument)) {
            logger->info(CMD_ORDERBOOK_CREATED, newInstrument);
        } else {
            logger->warn(CMD_ORDERBOOK_EXISTS, newInstrument);
        }
    };
}

void SystemLauncher::inputLoop()
{
    std::string input;
    while (running_) {
        logger->info(CMD_ENTER_COMMAND);
        std::getline(std::cin, input);
        if (commandHandlers.find(input) != commandHandlers.end()) {
            commandHandlers[input]();
        } else {
            logger->warn(CMD_UNKNOWN_COMMAND, input);
        }
    }
}