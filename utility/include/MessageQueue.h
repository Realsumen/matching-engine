#ifndef ORDERQUEUE_H
#define ORDERQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include "Message.hpp"

class MessageQueue {
public:
    MessageQueue();
    ~MessageQueue();

    // Disable copying and assignment
    MessageQueue(const MessageQueue&) = delete;
    MessageQueue& operator=(const MessageQueue&) = delete;

    // Pushing messages into a queue
    void push(Message&& msg);

    // Pop a message from the queue. If the queue is empty, block and wait
    bool pop(Message& msg);

    // Try to pop a message. If the queue is empty, return false immediately.
    bool tryPop(Message& msg);

    // Check if the queue is empty
    bool empty() const;

    // Get the queue size
    size_t size() const;

private:
    mutable std::mutex m_mutex;                // The mutex protecting the queue
    std::condition_variable m_condVar;         // Condition variables are used to notify consumers
    std::queue<Message> m_queue;               // Internal message queue
};

#endif // ORDERQUEUE_H
