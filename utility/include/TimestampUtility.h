#ifndef TIMESTAMPUTILITY_H
#define TIMESTAMPUTILITY_H

#include <chrono>
#include <string>

std::string timestampToString(const std::chrono::time_point<std::chrono::high_resolution_clock> &timestamp);
std::chrono::time_point<std::chrono::high_resolution_clock> currentTimestamp();
double durationInMilliseconds(const std::chrono::time_point<std::chrono::high_resolution_clock> &start,
                              const std::chrono::time_point<std::chrono::high_resolution_clock> &end);
bool isTimestampBefore(const std::chrono::time_point<std::chrono::high_resolution_clock> &lhs,
                       const std::chrono::time_point<std::chrono::high_resolution_clock> &rhs);
bool isTimestampAfter(const std::chrono::time_point<std::chrono::high_resolution_clock> &lhs,
                      const std::chrono::time_point<std::chrono::high_resolution_clock> &rhs);

#endif