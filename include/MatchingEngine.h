#ifndef MATCHINGENGINE_H
#define MATCHINGENGINE_H

#include <string>
#include <unordered_map>
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
    void addOrderBook(const std::string &instrument);
    void removeOrderBook(const std::string &instrument);

    // Process New Orders
    std::vector<Trade> processOrder(Order *order);

    // Cancel an order
    void cancelOrder(unsigned int orderId, const std::string &instrument);

    // Modify an order
    void modifyOrder(unsigned int orderId, const std::string &instrument, double newPrice, int newQuantity);

    // Get all trade records
    std::vector<Trade> getTrades() const;

    double getLastTradePrice(const std::string &instrument);

private:

    // Mapping from trading instrument to order book
    std::unordered_map<std::string, OrderBook *> orderBooks;

    // Mapping from instrument to the last traded price for the instrument
    std::unordered_map<std::string, double> instrumentToTradedPrice;
    
    // All trade records
    std::vector<Trade> trades;

    // assistant functions
    std::vector<Trade> processLimitOrder(Order *order);
    std::vector<Trade> processMarketOrder(Order *order);
    std::vector<Trade> processStopOrder(Order *order);

    std::vector<Trade> matchOrder(Order *order);

    // Get the order book for a specific trading instrument
    OrderBook *getOrderBook(const std::string &instrument);

};

#endif // MATCHINGENGINE_H
