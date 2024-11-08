// OrderQueue.cpp

#include "MessageQueue.h"

MessageQueue::MessageQueue() 
{
}

MessageQueue::~MessageQueue() 
{
}

void MessageQueue::push(Message&& msg) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.emplace(std::move(msg));
    }
    m_condVar.notify_one(); // Notify a readied consumer
}

bool MessageQueue::pop(Message& msg) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_condVar.wait(lock, [this]() { return !m_queue.empty(); }); // Block and wait until the queue is not empty

    if (!m_queue.empty()) {
        msg = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }
    return false;
}

bool MessageQueue::tryPop(Message& msg) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty()) {
        return false;
    }
    msg = std::move(m_queue.front());
    m_queue.pop();
    return true;
}

bool MessageQueue::empty() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
}

size_t MessageQueue::size() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
}
