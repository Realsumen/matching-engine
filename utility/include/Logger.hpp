#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <string>
#include "utility_config.hpp"

class Logger
{
public:
    static auto getLogger(const std::string& name = Utility_Config::Logging::DEFAULT_LOGGER_NAME) -> std::shared_ptr<spdlog::logger> {
        static bool is_thread_pool_initialized = false;
        if (!is_thread_pool_initialized) {
            initializeThreadPool();
            is_thread_pool_initialized = true;
        }
        
        auto logger = spdlog::get(name);
        if (!logger) {
            logger = createLogger(name);
        }
        return logger;
    }

private:
    static void initializeThreadPool() {
        spdlog::init_thread_pool(Utility_Config::Logging::LOG_QUEUE_SIZE, Utility_Config::Logging::LOG_THREADS);
    }
    
    static auto createLogger(const std::string& name) -> std::shared_ptr<spdlog::logger> {
        std::string log_file_name = name + ".log";

        auto async_file = std::make_shared<spdlog::async_logger>(
            name,
            std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_name, true),
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::block
        );
        spdlog::register_logger(async_file);
        spdlog::set_level(spdlog::level::info);
        async_file->set_pattern(Utility_Config::Logging::LOG_PATTERN);
        return async_file;
    }
};

#endif // LOGGER_HPP