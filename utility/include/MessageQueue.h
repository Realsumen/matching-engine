#ifndef ORDER_QUEUE_H
#define ORDER_QUEUE_H

#include <queue>
#include <chrono>
#include <condition_variable>
#include "Message.hpp"

class MessageQueue {
public:
    MessageQueue();
    ~MessageQueue();

    MessageQueue(const MessageQueue&) = delete;
    auto operator=(const MessageQueue&) -> MessageQueue& = delete;

    MessageQueue(MessageQueue&&) = delete;
    auto operator=(MessageQueue&&) -> MessageQueue& = delete;

    void push(Message&& msg);

    auto pop(Message& msg) -> bool;
    /* 
    * @param timeout: Used for testing scenario
    */

    auto tryPop(Message& msg) -> bool;

    auto empty() const -> bool;

    auto size() const -> size_t;

private:
    mutable std::mutex m_mutex;                // The mutex protecting the queue
    std::condition_variable m_condVar;         // Condition variables are used to notify consumers
    std::queue<Message> m_queue;               // Internal message queue
};

#endif // ORDER_QUEUE_H
