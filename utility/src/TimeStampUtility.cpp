#include "TimestampUtility.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

auto timestampToString(const std::chrono::system_clock::time_point &timestamp) -> std::string
{
    // Alter the timestamp to seconds and nanoseconds since the epoch
    std::time_t time = std::chrono::system_clock::to_time_t(timestamp);

    // Extract nanoseconds
    auto duration = timestamp.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - seconds);

    std::tm tm_time;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm_time, &time); // Thread-safe on Windows
#else
    localtime_r(&time, &tm_time); // Thread-safe on POSIX
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm_time, "%Y-%m-%d %H:%M:%S");

    oss << '.' << std::setw(9) << std::setfill('0') << nanoseconds.count();

    return oss.str();
}

auto currentTimestamp() -> std::chrono::system_clock::time_point
{
    return std::chrono::system_clock::now();
}

auto durationInNanoseconds(const std::chrono::system_clock::time_point &start,
                             const std::chrono::system_clock::time_point &end) -> double
{
    return std::chrono::duration<double, std::nano>(end - start).count();
}

auto durationInMilliseconds(const std::chrono::system_clock::time_point &start,
                              const std::chrono::system_clock::time_point &end) -> double
{
    return std::chrono::duration<double, std::milli>(end - start).count();
}

auto durationInSeconds(const std::chrono::system_clock::time_point &start,
                         const std::chrono::system_clock::time_point &end) -> double
{
    return std::chrono::duration<double>(end - start).count();
}

auto isTimestampBefore(const std::chrono::system_clock::time_point &lhs,
                       const std::chrono::system_clock::time_point &rhs) -> bool
{
    return lhs < rhs;
}

auto isTimestampAfter(const std::chrono::system_clock::time_point &lhs,
                      const std::chrono::system_clock::time_point &rhs) -> bool
{
    return lhs > rhs;
}