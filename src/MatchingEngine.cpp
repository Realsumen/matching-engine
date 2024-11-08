#include <iostream>
#include <vector>
#include <algorithm>
#include "MatchingEngine.h"
#include "Trade.h"
#include "Order.h"
#include "OrderBook.h"
#include "utility/include/IDGenerator.hpp"
#include "utility/include/OrderType.h"
#include "utility/include/TimestampUtility.h"

MatchingEngine::MatchingEngine()
{
    // Initialize Orderbook
    orderBooks = std::unordered_map<std::string, OrderBook *>();

    // Initialize the instrumentToTradedPrice
    instrumentToTradedPrice = std::unordered_map<std::string, double>();

    // Initialize the container for all the trade records
    trades = std::vector<Trade>();
}

void MatchingEngine::addOrderBook(const std::string &instrument)
{
    OrderBook *newOrderBook = new OrderBook(instrument);
    orderBooks[instrument] = newOrderBook;
}

void MatchingEngine::removeOrderBook(const std::string &instrument)
{
    auto it = orderBooks.find(instrument);
    if (it != orderBooks.end())
    {
        delete it->second;
        orderBooks.erase(it);
        std::cout << "Target instrument has been removed successfully." << std::endl;
    } 
    else 
    {
        std::cerr<<"Tried to remove a nonexisting instrument." << std::endl;
    }
}

std::vector<Trade> MatchingEngine::processOrder(Order *order)
{
    // All the trades happened
    std::vector<Trade> trades;

    OrderType type = order->getType();

    // Matching processing logic according to order type
    if (type == OrderType::LIMIT)
    {
        trades = MatchingEngine::processLimitOrder(order);
    }
    else if (type == OrderType::MARKET)
    {
        trades = MatchingEngine::processMarketOrder(order);
    }
    else if (type == OrderType::STOP)
    {
        trades = MatchingEngine::processStopOrder(order);
    }
    else
    {
        throw std::invalid_argument("Unknown Order Type.");
    }

    return trades;
}

void MatchingEngine::cancelOrder(unsigned int orderId, const std::string &instrument)
{
    OrderBook *orderBook = getOrderBook(instrument);

    OrderType type = orderBook->orderIdToOrderNode[orderId]->order->getType();

    if (type == OrderType::LIMIT)
    {
        orderBook->cancelLimitOrder(orderId);
    }
    else if (type == OrderType::STOP)
    {
        orderBook->cancelStopOrder(orderId);
    }
    else
    {
        std::cout<<"Does not support cancellation for market order "<<std::endl;
    }
    
}

void MatchingEngine::modifyOrder(unsigned int orderId, const std::string &instrument, double newPrice, int newQuantity)
{
    OrderBook *orderBook = getOrderBook(instrument);

    OrderType type = orderBook->orderIdToOrderNode[orderId]->order->getType();

    if (type == OrderType::LIMIT)
    {
        orderBook->modifyLimitOrder(orderId, newPrice, newQuantity);
    }
    else if (type == OrderType::STOP)
    {
        orderBook->modifyStopOrder(orderId, newPrice, newQuantity);
    }
    else
    {
        std::cout<<"Does not support modification for market order "<<std::endl;
    }
    
}

std::vector<Trade> MatchingEngine::getTrades() const
{
    return trades;
}

double MatchingEngine::getLastTradePrice(const std::string &instrument)
{
    return instrumentToTradedPrice[instrument];
}

std::vector<Trade> MatchingEngine::processLimitOrder(Order *order)
{   
    // If limit order does not enter the matching process, this vector is empty
    std::vector<Trade> trades;

    OrderBook *orderbook = getOrderBook(order->getAsset());

    if (order->isBuy())
    {   
        if (order->getPrice() >= orderbook->bestAskLevel->price)
        {
            int quantity = order->getQuantity();
            trades = matchOrder(order);
            int remainQuantity = order->getQuantity();
            std::cout << "There are " << quantity - remainQuantity << " units traded. Remaining " 
          << remainQuantity << " units enter into the order book." << std::endl;
        }
    } else{
        if (order->getPrice() <= orderbook->bestBidLevel->price)
        {
            int quantity = order->getQuantity();
            trades = matchOrder(order);
            int remainQuantity = order->getQuantity();
            std::cout << "There are " << quantity - remainQuantity << " units traded. Remaining " 
          << remainQuantity << " units enter into the order book." << std::endl;
        }
    }

    return trades;
}

std::vector<Trade> MatchingEngine::processMarketOrder(Order *order)
{
    std::vector<Trade> trades = matchOrder(order);

    if (order->getQuantity() > 0)
    {
        std::cerr << "Market Order " << order->getId() << " partially filled. remaining Quantity: " << order->getQuantity() << "\n";
    }

    delete order;

    return;
}

std::vector<Trade> MatchingEngine::processStopOrder(Order *order)
{
    bool isBuy = order->isBuy();
    double price = order->getPrice();

    if (isBuy)
    {
        if (price)
        {
            /* code */
        }
        
    }
    
    return std::vector<Trade>();
}

std::vector<Trade> MatchingEngine::matchOrder(Order *order)
{   

    std::vector<Trade> trades;
    // Find the asset of the order
    std::string instrument = order->getAsset();
    // Get the orderbook for the order
    OrderBook *orderbook = getOrderBook(instrument);
    int remainingQuantity = order->getQuantity();
    OrderBook::PriceLevel *bestLevel = nullptr;
    bool is_buy = order->isBuy();

    // Determine the order's opposite direction for the BBO levels.
    bestLevel = is_buy ? orderbook->bestAskLevel:orderbook->bestBidLevel;

    while (remainingQuantity > 0 && bestLevel != nullptr)
    {   
        // Iterate PriceLevel one by one
        OrderBook::OrderNode *oppositeOrderNode = bestLevel -> headOrder;
        double tradedPrice = bestLevel->price;
        
        if (order->getType() == OrderType::LIMIT)
        {
            // For a limit order that enters the matching process, check the price condition
            if (is_buy && order->getPrice() < tradedPrice) {
                break;  // Buy order price lower than bestAsk
            }
            if (!is_buy && order->getPrice() > tradedPrice) {
                break;  // Sell order price higher than bestBid
            }
        }

        while (remainingQuantity > 0 && oppositeOrderNode != nullptr)
        {
            // Iterate all the OrderNodes within a PriceLevel
            Order *oppositeOrder = oppositeOrderNode -> order;
            int oppositeQuantity = oppositeOrder->getQuantity();
            int tradedQuantity = std::min(oppositeQuantity, remainingQuantity);
            unsigned int newTradeId = IDGenerator::getInstance().getNextTradeID();

            // Generate a new Trade instance
            Trade trade(newTradeId, 
                order->getId(), 
                oppositeOrder->getId(), 
                instrument, 
                tradedPrice, 
                tradedQuantity);
            trades.push_back(trade);

            // Update quantity for taker and maker, and current PriceLevel
            remainingQuantity -= tradedQuantity;
            oppositeOrder->setQuantity(oppositeQuantity - tradedQuantity);
            bestLevel->totalQuantity -= tradedQuantity;
            
            // Save the next oppositeOrderNode
            OrderBook::OrderNode *nextOppositeOrderNode = oppositeOrderNode->next;

            if (oppositeOrder->getQuantity() == 0) {
                // If the opposite order's quantity becomes zero, remove it and its node
                orderbook -> orderIdToOrderNode.erase(oppositeOrder->getId());
                orderbook -> removeOrderNodeFromBook(oppositeOrderNode);
                delete oppositeOrder;
            }

            // Move to the Next oppositeOrderNode
            oppositeOrderNode = nextOppositeOrderNode;
        }
    }

    for (size_t i = 0; i < trades.size(); ++i)
    {
        Trade &trade = trades[i];
        bool isLastTrade = (i == trades.size() - 1);

        if (!isLastTrade)
        {
            // Before the last trades, all the status for opposite orders are SUCCESS
            if (is_buy) 
            {
                trade.setSellOrderStatus(TradeStatus::SUCCESS);
                trade.setBuyOrderStatus(TradeStatus::PARTIALLY_FILLED);
            } else {
                trade.setBuyOrderStatus(TradeStatus::SUCCESS);
                trade.setSellOrderStatus(TradeStatus::PARTIALLY_FILLED);
            }
        } else
        {   
            // For the last trade, check if the opposite order is in orderIdToOrderNode
            // If yes, then the order is success and opposite order is PARTIALLY_FILLED
            if (is_buy) 
            {
                bool opposieOrderInMap = orderbook->orderIdToOrderNode.find(trade.getSellOrderId()) != orderbook->orderIdToOrderNode.end();
                trade.setSellOrderStatus(opposieOrderInMap ? TradeStatus::PARTIALLY_FILLED : TradeStatus::SUCCESS);
                trade.setBuyOrderStatus(remainingQuantity != 0 ? TradeStatus::PARTIALLY_FILLED : TradeStatus::SUCCESS);
            } else {
                bool opposieOrderInMap = orderbook->orderIdToOrderNode.find(trade.getBuyOrderId()) != orderbook->orderIdToOrderNode.end();
                trade.setBuyOrderStatus(opposieOrderInMap ? TradeStatus::PARTIALLY_FILLED : TradeStatus::SUCCESS);
                trade.setSellOrderStatus(remainingQuantity != 0 ? TradeStatus::PARTIALLY_FILLED : TradeStatus::SUCCESS);
            }
        }
    }

    // Update the quantity for the order
    order->setQuantity(remainingQuantity);

    return trades;
}

OrderBook *MatchingEngine::getOrderBook(const std::string &instrument)
{   
    auto it = orderBooks.find(instrument);
    return (it != orderBooks.end()) ? it->second : nullptr;
}


