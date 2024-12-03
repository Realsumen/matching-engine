// OrderQueue.cpp

#include "MessageQueue.h"

MessageQueue::MessageQueue() 
= default;

MessageQueue::~MessageQueue() 
= default;

void MessageQueue::push(Message&& msg) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.emplace(std::move(msg));
    }
    m_condVar.notify_one(); // Notify a readied consumer
}

auto MessageQueue::pop(Message& msg) -> bool {
    // The incoming parameter msg is used to store the information popped out of the queue
    std::unique_lock<std::mutex> lock(m_mutex);
    m_condVar.wait(lock, [this]() { return !m_queue.empty(); }); // Block and wait until the queue is not empty

    if (!m_queue.empty()) {
        msg = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }
    return false;
}

auto MessageQueue::tryPop(Message& msg) -> bool {
    // The incoming parameter msg is used to store the information popped out of the queue
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty()) {
        return false;
    }
    msg = std::move(m_queue.front());
    m_queue.pop();
    return true;
}

auto MessageQueue::empty() const -> bool {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
}

auto MessageQueue::size() const -> size_t {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
}
