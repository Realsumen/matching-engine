// IDGenerator.h
#ifndef MATCHING_ENGINE_ID_GENERATOR_H
#define MATCHING_ENGINE_ID_GENERATOR_H

#include <atomic>

class IDGenerator
{
public:
    // Obtain singleton instance
    static auto getInstance() -> IDGenerator &
    {
        static IDGenerator instance;
        return instance;
    }

    // Prohibit copy and assignment
    IDGenerator(const IDGenerator &) = delete;
    auto operator=(const IDGenerator &) -> IDGenerator & = delete;

    auto getNextOrderID() -> unsigned int
    {
        return orderIDCounter.fetch_add(1, std::memory_order_relaxed);
    }

    auto getNextTradeID() -> unsigned int
    {
        return tradeIDCounter.fetch_add(1, std::memory_order_relaxed);
    }

    void reset()
    {
        orderIDCounter.store(1, std::memory_order_relaxed);
        tradeIDCounter.store(1, std::memory_order_relaxed);
    }

private:
    // Private constructor to prevent external instantiation
    IDGenerator() : orderIDCounter(1), tradeIDCounter(1) {}

    std::atomic<unsigned int> orderIDCounter;
    std::atomic<unsigned int> tradeIDCounter;
};

#endif // MATCHING_ENGINE_ID_GENERATOR_H