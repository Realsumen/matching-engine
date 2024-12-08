#ifndef MATCHING_ENGINE_H
#define MATCHING_ENGINE_H

#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Order.h"
#include "Trade.h"
#include "OrderBook.h"

class MatchingEngine
{
public:
    MatchingEngine();
    ~MatchingEngine();

    MatchingEngine(const MatchingEngine&) = delete;
    auto operator=(const MatchingEngine&) -> MatchingEngine& = delete;

    MatchingEngine(const MatchingEngine&&) = delete;
    auto operator=(const MatchingEngine&&) -> MatchingEngine& = delete;

    // Add a new Instrument or Remove an existing Instrument
    auto createNewOrderBook(const std::string &instrument) -> bool;
    void removeOrderBook(const std::string &instrument);

    auto processNewOrder(Order *order) -> std::vector<Trade>;

    void cancelOrder(unsigned int orderId, const std::string &instrument);

    void modifyOrder(unsigned int orderId, const std::string &instrument, double newPrice, int newQuantity);

    [[nodiscard]] auto getTrades() -> std::vector<Trade>;

    auto getLastTradePrice(const std::string &instrument) -> double;

    [[nodiscard]] auto getOrderBookForRead(const std::string &instrument) -> const OrderBook*;

    auto hasOrder(const std::string& instrument, unsigned int orderId) -> bool;

    auto hasInstrument(const std::string& instrument) -> bool;

    auto hasOrderId(unsigned int orderId) -> bool;

private:

    std::shared_mutex orderBooksMutex;

    std::unordered_set<unsigned int> globalOrderIds;

    std::unordered_map<std::string, OrderBook *> orderBooks;

    std::unordered_map<std::string, double> instrumentToTradedPrice;
    
    std::vector<Trade> trades;

    auto processLimitOrder(Order *order) -> std::vector<Trade>;
    auto processMarketOrder(Order *order) -> std::vector<Trade>;
    auto processStopOrder(Order *order) -> std::vector<Trade>;

    auto matchOrder(Order *order) -> std::vector<Trade>;

    auto getOrderBook(const std::string &instrument) -> OrderBook *;

};

#endif // MATCHING_ENGINE_H
