// IDGenerator.h
#ifndef IDGENERATOR_H
#define IDGENERATOR_H

#include <atomic>
#include <string>
#include <sstream>
#include <iomanip>

class IDGenerator
{
public:
    // Obtain singleton instance
    static IDGenerator &getInstance()
    {
        static IDGenerator instance;
        return instance;
    }

    // Prohibit copy and assignment
    IDGenerator(const IDGenerator &) = delete;
    IDGenerator &operator=(const IDGenerator &) = delete;

    unsigned int getNextOrderID()
    {
        return orderIDCounter.fetch_add(1, std::memory_order_relaxed);
    }

    unsigned int getNextTradeID()
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

#endif // IDGENERATOR_H