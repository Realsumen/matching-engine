#ifndef MATCHINGENGINE_H
#define MATCHINGENGINE_H

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
    // Constructor, initializes the matching engine
    MatchingEngine();

    // Disable copying and assignment
    MatchingEngine(const MatchingEngine&) = delete;
    MatchingEngine& operator=(const MatchingEngine&) = delete;

    // Add a new Instrument or Remove an existing Instrument
    void createNewOrderBook(const std::string &instrument);
    void removeOrderBook(const std::string &instrument);

    std::vector<Trade> processNewOrder(Order *order);

    void cancelOrder(unsigned int orderId, const std::string &instrument);

    void modifyOrder(unsigned int orderId, const std::string &instrument, double newPrice, int newQuantity);

    std::vector<Trade> getTrades() const;

    double getLastTradePrice(const std::string &instrument);

    const OrderBook *getOrderBookForRead(const std::string &instrument) const;

    bool hasOrder(std::string instrument, unsigned int orderId);

    bool hasInstrument(std::string instrument);

    bool hasOrderId(unsigned int orderId);

private:

    std::unordered_set<unsigned int> globalOrderIds;

    std::unordered_map<std::string, OrderBook *> orderBooks;

    std::unordered_map<std::string, double> instrumentToTradedPrice;
    
    std::vector<Trade> trades;

    // assistant functions
    std::vector<Trade> processLimitOrder(Order *order);
    std::vector<Trade> processMarketOrder(Order *order);
    std::vector<Trade> processStopOrder(Order *order);

    std::vector<Trade> matchOrder(Order *order);

    OrderBook *getOrderBook(const std::string &instrument);

};

#endif // MATCHINGENGINE_H
