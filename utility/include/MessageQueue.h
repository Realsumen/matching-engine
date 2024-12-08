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
    bool pop(Message& msg);
    bool tryPop(Message& msg);
    bool empty() const;
    size_t size() const;

    void shutdown();
    bool isShutdown() const;

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_condVar;
    std::queue<Message> m_queue;
    bool m_shutdown = false;
};

#endif // ORDER_QUEUE_H
