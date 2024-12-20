#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include <string>
#include <unordered_map>
#include <map>
#include <stack>
#include "Order.h"

enum class Side
{
    BUY,
    SELL,
    UNDEFINED
};

class OrderBook
{
public:
    // Constructor
    explicit OrderBook(std::string instrument);

    using CrossCallback = std::function<void(Order*)>;

    void setCrossCallback(CrossCallback callback);

    // Destructor
    ~OrderBook();

    // Disable copying and assignment
    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;

    // Cancel Orders
    void cancelLimitOrder(unsigned int orderId);
    void cancelStopOrder(unsigned int orderId);

    // Modify Orders
    void modifyLimitOrder(unsigned int orderId, double newPrice, int newQuantity);
    void modifyStopOrder(unsigned int orderId, double newPrice, int newQuantity);

    // Get the BBA
    [[nodiscard]] Order *getBestBid() const;
    [[nodiscard]] Order *getBestAsk() const;

    // Print the OrderBook - Print all the price layers
    void printOrderBook() const;

    // Print the OrderBook with designated price range
    void printOrderBook(double minPrice, double maxPrice) const;

    // Print the OrderBook with designated top N levels
    void printOrderBook(int depth) const;

    friend class MatchingEngine;

private:
    // Node for each order at the same price level
    struct OrderNode
    {
        Order *order = nullptr;
        OrderNode *prev = nullptr; // The previous order at the same price level
        OrderNode *next = nullptr; // The next order at the same price level
    };

    // Node for Price Level, it contains all the orders and total amount for the price
    struct PriceLevel
    {
        double price = -1;
        int totalQuantity = -1;
        Side side = Side::UNDEFINED;
        OrderNode *headOrder = nullptr;  // Link List Header
        OrderNode *tailOrder = nullptr;  // Link List Tail
        PriceLevel *prevPrice = nullptr; // Previous Price level
        PriceLevel *nextPrice = nullptr; // Next Price Level
    };

    std::string instrument;
    CrossCallback crossCallback;

    // BBO
    PriceLevel *bestBidLevel;
    PriceLevel *bestAskLevel;

    // Mapping from price to price level, for quick access
    std::unordered_map<double, PriceLevel *> priceToPriceLevel;
    std::map<double, OrderNode *> priceToStopOrder;

    // Mapping from order ID to the Order node, for quick cancellation and order modification
    std::unordered_map<unsigned int, OrderNode *> orderIdToOrderNode;

    // Improve memory management and efficiency
    std::stack<OrderNode *> emptyOrderNodeStack;
    std::stack<PriceLevel *> emptyPriceLevelStack;
    
    void addPriceLevel(PriceLevel *priceLevel, PriceLevel *&bestLevel);
    void addLimitOrderToBook(Order *order);
    void addStopOrderToBook(Order *order);
    void removeOrderNodeFromBook(OrderNode *orderNode);
    void removePriceFromBook(PriceLevel *priceLevel);
    void updateBestPrices();

    // Get an empty node from the emptyOrderNodeStack
    OrderNode *getOrderNode();
    void releaseOrderNode(OrderNode *node);
    // Get an empty PriceLevel from the emptyPriceLevelStack
    PriceLevel *getPriceLevel();
    void releasePriceLevel(PriceLevel *level);

    // clear and optimize memory layout
    void cleanup();

    // assistant function to print a specific price level
    void printPriceLevel(const PriceLevel *start, int depth) const;
};

#endif // ORDER_BOOK_H
