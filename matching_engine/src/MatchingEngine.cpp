#include <iostream>
#include <vector>
#include "MatchingEngine.h"
#include "Logger.hpp"
#include "matching_engine_config.hpp"
#include "Trade.h"
#include "Order.h"
#include "OrderBook.h"
#include "IDGenerator.hpp"
#include "OrderType.h"

MatchingEngine::MatchingEngine()
{
    orderBooks = std::unordered_map<std::string, OrderBook *>();

    instrumentToTradedPrice = std::unordered_map<std::string, double>();

    trades = std::vector<Trade>();
}

MatchingEngine::~MatchingEngine()
{

}

void MatchingEngine::createNewOrderBook(const std::string &instrument)
{
    auto *newOrderBook = new OrderBook(instrument);
    newOrderBook->setCrossCallback(
        [this](Order* order) { this->processNewOrder(order); }
    );
    orderBooks[instrument] = newOrderBook;
}

void MatchingEngine::removeOrderBook(const std::string &instrument)
{
    auto iter = orderBooks.find(instrument);
    if (iter != orderBooks.end())
    {
        delete iter->second;
        orderBooks.erase(iter);
        std::cout << "Target instrument has been removed successfully." << '\n';
    } 
    else 
    {
        std::cerr<<"Tried to remove a nonexistent instrument." << '\n';
    }
}

auto MatchingEngine::processNewOrder(Order *order) -> std::vector<Trade>
{
    std::vector<Trade> trades;

    const OrderType type = order->getType();

    globalOrderIds.insert(order->getId());

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


    auto logger = Logger::getLogger(Matching_Engine_Config::MathingEngine::LOGGER_NAME);
    for (const auto& trade : trades) {
        logger->info(trade.toString());
    }
    this->trades.insert(this->trades.end(), trades.begin(), trades.end());
    
    return trades;
}

void MatchingEngine::cancelOrder(const unsigned int orderId, const std::string &instrument)
{
    OrderBook *orderBook = getOrderBook(instrument);

    if (const OrderType type = orderBook->orderIdToOrderNode[orderId]->order->getType(); type == OrderType::LIMIT)
    {
        orderBook->cancelLimitOrder(orderId);
    }
    else if (type == OrderType::STOP)
    {
        orderBook->cancelStopOrder(orderId);
    }
    else
    {
        std::cout<<"Does not support cancellation for market order " << '\n';
    }
    
}

void MatchingEngine::modifyOrder(unsigned int orderId, const std::string &instrument, double newPrice, int newQuantity)
{
    OrderBook *orderBook = getOrderBook(instrument);

    if (OrderType type = orderBook->orderIdToOrderNode[orderId]->order->getType(); type == OrderType::LIMIT)
    {
        orderBook->modifyLimitOrder(orderId, newPrice, newQuantity);
    }
    else if (type == OrderType::STOP)
    {
        orderBook->modifyStopOrder(orderId, newPrice, newQuantity);
    }
    else
    {
        std::cout << "Does not support modification for market order " << '\n';
    }
    
}

auto MatchingEngine::getTrades()  -> std::vector<Trade>
{
    return trades;
}

auto MatchingEngine::getLastTradePrice(const std::string &instrument) -> double
{
    return instrumentToTradedPrice[instrument];
}

auto MatchingEngine::getOrderBookForRead(const std::string &instrument) const -> const OrderBook *
{
    const auto iter = orderBooks.find(instrument);
    return (iter != orderBooks.end()) ? iter->second : nullptr;
}

auto MatchingEngine::hasOrder(const std::string& instrument, unsigned int orderId) -> bool
{   
    OrderBook *book = orderBooks[instrument];
    return (book->orderIdToOrderNode.find(orderId) != book->orderIdToOrderNode.end());
}

auto MatchingEngine::hasInstrument(const std::string& instrument) -> bool
{
    if (const auto iter = orderBooks.find(instrument); iter == orderBooks.end())
    {
        return false;
    }
    {
        return true;
    }
}

auto MatchingEngine::hasOrderId(const unsigned int orderId) -> bool
{
    return (globalOrderIds.find(orderId) != globalOrderIds.end());
}

auto MatchingEngine::processLimitOrder(Order *order) -> std::vector<Trade>
{   
    // If limit order does not enter the matching process, this vector is empty
    std::vector<Trade> trades;
    OrderBook *orderBook = getOrderBook(order->getAsset());

    const bool isBuy = order->isBuy();
    const OrderBook::PriceLevel *bestLevel = isBuy ? orderBook->bestAskLevel : orderBook->bestBidLevel;
    bool shouldMatch = false;
    if (bestLevel != nullptr) {
        shouldMatch = isBuy ? (order->getPrice() >= bestLevel->price)
                              : (order->getPrice() <= bestLevel->price);
    }

    if (shouldMatch)
    {
        trades = matchOrder(order);
    }

    if (order->getQuantity() > 0)
    {
        orderBook->addLimitOrderToBook(order);
    }

    return trades;
}

auto MatchingEngine::processMarketOrder(Order *order) -> std::vector<Trade>
{
    const int originalQuantity = order->getQuantity();
    std::vector<Trade> trades = matchOrder(order);
    const int newQuantity = order->getQuantity();

    if (order->getQuantity() > 0)
    {
        std::cerr << "Market Order " << order->getId() << " partially filled." 
        << originalQuantity - newQuantity << "filled." << "remaining Quantity: " 
        << newQuantity << "\n";
    }
    delete order;
    return trades;
}

auto MatchingEngine::processStopOrder(Order *order) -> std::vector<Trade>
{
    // TODO: Stop Order logic
    bool isBuy = order->isBuy();
    double price = order->getPrice();

    if (isBuy)
    {
        if (price)
        {
            /* code */
        }
        
    }
    
    return {};
}


auto MatchingEngine::matchOrder(Order *order) -> std::vector<Trade>
{   
    // TODO: Trigger logic for Stop Order
    std::vector<Trade> trades;

    const std::string instrument = order->getAsset();
    OrderBook *orderBook = getOrderBook(instrument);
    int remainingQuantity = order->getQuantity();
    bool is_buy = order->isBuy();

    // Determine the best opposite quote
    OrderBook::PriceLevel *bestLevel = is_buy ? orderBook->bestAskLevel:orderBook->bestBidLevel;

    while (remainingQuantity > 0 && bestLevel != nullptr)
    {
        // Iterate PriceLevel one by one
        OrderBook::OrderNode *oppositeOrderNode = bestLevel -> headOrder;
        double tradedPrice = bestLevel->price;

        if (order->getType() == OrderType::LIMIT)
        {
            // For a limit order that enters the matching process, check the price condition
            if ((is_buy && order->getPrice() < tradedPrice) ||
                (!is_buy && order->getPrice() > tradedPrice))
            {
                break;
            }
        }

        while (remainingQuantity > 0 && oppositeOrderNode != nullptr)
        {
            // Iterate all the OrderNodes within a PriceLevel
            Order *oppositeOrder = oppositeOrderNode -> order;
            int oppositeQuantity = oppositeOrder->getQuantity();
            const int tradedQuantity = std::min(oppositeQuantity, remainingQuantity);
            const unsigned int newTradeId = IDGenerator::getInstance().getNextTradeID();

            Trade trade(newTradeId,
                is_buy ? order->getId() : oppositeOrder->getId(),
                is_buy ? oppositeOrder->getId() : order->getId(),
                instrument,
                tradedPrice,
                tradedQuantity);
            trades.push_back(trade);

            // Update taker
            remainingQuantity -= tradedQuantity;

            // Update maker
            if (oppositeOrder->getQuantity() == tradedQuantity) {
                // If the opposite orderNode's quantity is 0, we need to remove the node and delete the order
                // When we remove the order, it will perform level quantity update and level delete automatically
                orderBook -> orderIdToOrderNode.erase(oppositeOrder->getId());
                OrderBook::OrderNode *nextNode = oppositeOrderNode->next;
                orderBook -> removeOrderNodeFromBook(oppositeOrderNode);
                delete oppositeOrder;
                oppositeOrderNode = nextNode;

                if (oppositeOrderNode == nullptr)
                {
                    bestLevel = is_buy ? orderBook->bestAskLevel : orderBook->bestBidLevel;
                }
            }
            else
            {
                // Update quantity of the bestLevel
                bestLevel->totalQuantity -= tradedQuantity;
                oppositeOrder->setQuantity(oppositeQuantity - tradedQuantity);
            }
        }
    }

    size_t tradeCount = trades.size();
    for (size_t i = 0; i < tradeCount; ++i)
    {
        Trade &trade = trades[i];
        bool isLastTrade = (i == tradeCount - 1);

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
                const bool oppositeOrderInMap = orderBook->orderIdToOrderNode.find(trade.getSellOrderId()) != orderBook->orderIdToOrderNode.end();
                trade.setSellOrderStatus(oppositeOrderInMap ? TradeStatus::PARTIALLY_FILLED : TradeStatus::SUCCESS);
                trade.setBuyOrderStatus(remainingQuantity != 0 ? TradeStatus::PARTIALLY_FILLED : TradeStatus::SUCCESS);
            } else {
                const bool oppositeOrderInMap = orderBook->orderIdToOrderNode.find(trade.getBuyOrderId()) != orderBook->orderIdToOrderNode.end();
                trade.setBuyOrderStatus(oppositeOrderInMap ? TradeStatus::PARTIALLY_FILLED : TradeStatus::SUCCESS);
                trade.setSellOrderStatus(remainingQuantity != 0 ? TradeStatus::PARTIALLY_FILLED : TradeStatus::SUCCESS);
            }
        }
    }

    order->setQuantity(remainingQuantity);

    if (!trades.empty()) {
        instrumentToTradedPrice[instrument] = trades.back().getPrice();
    }
    return trades;
}

auto MatchingEngine::getOrderBook(const std::string &instrument) -> OrderBook*
{
    const auto iter = orderBooks.find(instrument);
    return (iter != orderBooks.end()) ? iter->second : nullptr;
}


