#ifndef UTILITY_CONFIG_HPP
#define UTILITY_CONFIG_HPP

namespace Utility_Config {
    constexpr int DEFAULT_TIMEOUT_MS = 5000;
    constexpr int MAX_RETRIES = 3;

    namespace Trade {
        constexpr int DEFAULT_ALIGNMENT_DISPLAY = 20;
        constexpr int DEFAULT_PRECISION_DISPLAY = 2;
    }

    namespace Logging {
        constexpr int LOG_QUEUE_SIZE = 8192;
        constexpr int LOG_THREADS = 1;
        constexpr auto DEFAULT_LOGGER_NAME = "default_logger";
        constexpr auto LOG_PATTERN = "[%Y-%m-%d %H:%M:%S.%e] [%l] %v";
    }
}

#endif // UTILITY_CONFIG_HPP