#ifndef TIMESTAMP_UTILITY_H
#define TIMESTAMP_UTILITY_H

#include <chrono>
#include <string>

auto timestampToString(const std::chrono::system_clock::time_point &timestamp) -> std::string;
auto currentTimestamp() -> std::chrono::system_clock::time_point;
auto durationInNanoseconds(const std::chrono::system_clock::time_point &start,
                             const std::chrono::system_clock::time_point &end) -> double;
auto durationInMilliseconds(const std::chrono::system_clock::time_point &start,
                              const std::chrono::system_clock::time_point &end) -> double;
auto durationInSeconds(const std::chrono::system_clock::time_point &start,
                         const std::chrono::system_clock::time_point &end) -> double;
auto isTimestampBefore(const std::chrono::system_clock::time_point &lhs,
                       const std::chrono::system_clock::time_point &rhs) -> bool;
auto isTimestampAfter(const std::chrono::system_clock::time_point &lhs,
                      const std::chrono::system_clock::time_point &rhs) -> bool;

#endif // TIMESTAMP_UTILITY_H