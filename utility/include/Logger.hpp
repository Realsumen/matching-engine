#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>
#include <string>
#include "utility_config.hpp"

class Logger
{
public:
    static auto getLogger(const std::string& name = Utility_Config::Logging::DEFAULT_LOGGER_NAME, const bool toConsole = false) -> std::shared_ptr<spdlog::logger> {
        std::call_once(init_flag_, initializeThreadPool);
        
        auto logger = spdlog::get(name);
        if (!logger) {
            logger = createLogger(name, toConsole);
        }
        return logger;
    }

private:
    static inline std::once_flag init_flag_;

    static void initializeThreadPool() {
        spdlog::init_thread_pool(Utility_Config::Logging::LOG_QUEUE_SIZE, Utility_Config::Logging::LOG_THREADS);
    }
    
    static auto createLogger(const std::string& name, const bool toConsole) -> std::shared_ptr<spdlog::logger> {
        std::string log_file_name = name + ".log";

        std::vector<spdlog::sink_ptr> sinks;

        // 始终添加文件 Sink
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_name, true);
        file_sink->set_pattern(Utility_Config::Logging::LOG_PATTERN);
        sinks.emplace_back(file_sink);

        // 根据参数决定是否添加控制台 Sink
        if (toConsole) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_pattern(Utility_Config::Logging::LOG_PATTERN);
            sinks.emplace_back(console_sink);
        }

        // 创建异步 Logger
        auto async_logger = std::make_shared<spdlog::async_logger>(
            name,
            sinks.begin(),
            sinks.end(),
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::block
        );

        spdlog::register_logger(async_logger);
        async_logger->set_level(spdlog::level::info);
        async_logger->flush_on(spdlog::level::info);
        return async_logger;
    }
};


#endif // LOGGER_HPP