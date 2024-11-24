#ifndef TIMESTAMPUTILITY_H
#define TIMESTAMPUTILITY_H

#include <chrono>
#include <string>

std::string timestampToString(const std::chrono::system_clock::time_point &timestamp);
std::chrono::system_clock::time_point currentTimestamp();
double durationInNanoseconds(const std::chrono::system_clock::time_point &start,
                             const std::chrono::system_clock::time_point &end);
double durationInMilliseconds(const std::chrono::system_clock::time_point &start,
                              const std::chrono::system_clock::time_point &end);
double durationInSeconds(const std::chrono::system_clock::time_point &start,
                         const std::chrono::system_clock::time_point &end);
bool isTimestampBefore(const std::chrono::system_clock::time_point &lhs,
                       const std::chrono::system_clock::time_point &rhs);
bool isTimestampAfter(const std::chrono::system_clock::time_point &lhs,
                      const std::chrono::system_clock::time_point &rhs);

#endif