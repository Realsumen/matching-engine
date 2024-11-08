#include "TimestampUtility.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

std::string timestampToString(const std::chrono::time_point<std::chrono::high_resolution_clock> &timestamp)
{
    // Alter the timestamp to seconds and nanoseconds since the epoch
    auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(timestamp).time_since_epoch();
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp.time_since_epoch()) % 1'000'000'000;

    // Using system_clock to obtain time
    auto systemTime = std::chrono::time_point<std::chrono::system_clock>(seconds);
    std::time_t time = std::chrono::system_clock::to_time_t(systemTime);

    // Format to y-m-d H-M-S
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");

    // add nanosceonds
    oss << '.' << std::setw(9) << std::setfill('0') << nanoseconds.count();

    return oss.str();
}

std::chrono::time_point<std::chrono::high_resolution_clock> currentTimestamp()
{
    return std::chrono::high_resolution_clock::now();
}

double durationInNanoseconds(const std::chrono::time_point<std::chrono::high_resolution_clock> &start,
                              const std::chrono::time_point<std::chrono::high_resolution_clock> &end)
{
    return std::chrono::duration<double, std::nano>(end - start).count();
}



double durationInMilliseconds(const std::chrono::time_point<std::chrono::high_resolution_clock> &start,
                              const std::chrono::time_point<std::chrono::high_resolution_clock> &end)
{
    return std::chrono::duration<double, std::milli>(end - start).count();
}

double durationInSeconds(const std::chrono::time_point<std::chrono::high_resolution_clock> &start,
                              const std::chrono::time_point<std::chrono::high_resolution_clock> &end)
{
    return std::chrono::duration<double>(end - start).count();
}

bool isTimestampBefore(const std::chrono::time_point<std::chrono::high_resolution_clock> &lhs,
                       const std::chrono::time_point<std::chrono::high_resolution_clock> &rhs)
{
    return lhs < rhs;
}

bool isTimestampAfter(const std::chrono::time_point<std::chrono::high_resolution_clock> &lhs,
                      const std::chrono::time_point<std::chrono::high_resolution_clock> &rhs)
{
    return lhs > rhs;
}