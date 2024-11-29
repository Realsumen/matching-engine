#ifndef ORDERQUEUE_H
#define ORDERQUEUE_H

#include <queue>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include "Message.hpp"

class MessageQueue {
public:
    MessageQueue();
    ~MessageQueue();

    MessageQueue(const MessageQueue&) = delete;
    MessageQueue& operator=(const MessageQueue&) = delete;

    void push(Message&& msg);

    bool pop(Message& msg, std::chrono::milliseconds timeout = std::chrono::milliseconds(100));

    bool tryPop(Message& msg);

    bool empty() const;

    size_t size() const;

private:
    mutable std::mutex m_mutex;                // The mutex protecting the queue
    std::condition_variable m_condVar;         // Condition variables are used to notify consumers
    std::queue<Message> m_queue;               // Internal message queue
};

#endif // ORDERQUEUE_H
