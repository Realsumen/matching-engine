#include <gtest/gtest.h>
#include <iostream>
#include "Order.h"
#include "MatchingEngine.h"
#include "IDGenerator.hpp"

TEST(MatchingEngineTest, AddLimitOrder)
{
    IDGenerator::getInstance().reset();
    MatchingEngine engine;
    std::string instrument = "AAPL";
    engine.addOrderBook(instrument);

    // Creates a limit buy order
    unsigned int buyOrderId = IDGenerator::getInstance().getNextOrderID();
    Order *buyOrder = new Order(buyOrderId, instrument, 150.0, 100, true, OrderType::LIMIT);
    std::vector<Trade> trades = engine.processNewOrder(buyOrder);

    EXPECT_TRUE(trades.empty());

    Order *bestBid = engine.getOrderBookForRead(instrument)->getBestBid();
    ASSERT_NE(bestBid, nullptr);
    EXPECT_DOUBLE_EQ(bestBid->getPrice(), 150.0);

    // Creats a limit sell order, which has price crossed with the best Bid
    unsigned int sellOrderId = IDGenerator::getInstance().getNextOrderID();
    Order *sellOrder = new Order(sellOrderId, instrument, 150.0, 120, false, OrderType::LIMIT);
    trades = engine.processNewOrder(sellOrder);

    // There should be a transaction
    ASSERT_EQ(trades.size(), buyOrderId);
    EXPECT_EQ(trades[0].getBuyOrderId(), buyOrderId);
    EXPECT_EQ(trades[0].getSellOrderId(), sellOrderId);
    EXPECT_EQ(trades[0].getAsset(), instrument);
    EXPECT_DOUBLE_EQ(trades[0].getPrice(), 150.0);
    EXPECT_EQ(trades[0].getQuantity(), 100);

    // Current BBO should be empty
    EXPECT_EQ(engine.getOrderBookForRead(instrument)->getBestBid(), nullptr);
    EXPECT_EQ(engine.getOrderBookForRead(instrument)->getBestAsk(), sellOrder);
}

TEST(MatchingEngineTest, MultiOrderProcessing)
{
    IDGenerator::getInstance().reset();
    MatchingEngine engine;
    std::string instrument = "AAPL";
    engine.addOrderBook(instrument);

    // Add Limit Order on Bid side
    unsigned int buyOrderId1 = IDGenerator::getInstance().getNextOrderID();
    Order *buyOrder1 = new Order(buyOrderId1, instrument, 150.0, 100, true, OrderType::LIMIT);
    engine.processNewOrder(buyOrder1);

    unsigned int buyOrderId2 = IDGenerator::getInstance().getNextOrderID();
    Order *buyOrder2 = new Order(buyOrderId2, instrument, 155.0, 50, true, OrderType::LIMIT);
    engine.processNewOrder(buyOrder2);

    Order *bestBid = engine.getOrderBookForRead(instrument)->getBestBid();
    ASSERT_NE(bestBid, nullptr);
    EXPECT_DOUBLE_EQ(bestBid->getPrice(), 155.0);
    EXPECT_EQ(bestBid->getQuantity(), 50);

    // Add Limit Order on Ask side
    unsigned int sellOrderId1 = IDGenerator::getInstance().getNextOrderID();
    Order *sellOrder1 = new Order(sellOrderId1, instrument, 152.0, 120, false, OrderType::LIMIT);
    engine.processNewOrder(sellOrder1);

    unsigned int sellOrderId2 = IDGenerator::getInstance().getNextOrderID();
    Order *sellOrder2 = new Order(sellOrderId2, instrument, 160.0, 25, false, OrderType::LIMIT);
    engine.processNewOrder(sellOrder2);

    std::vector<Trade> trades = engine.getTrades();
    std::cout << trades[0].toString() << std::endl;
    
    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].getBuyOrderId(), buyOrderId2);
    EXPECT_DOUBLE_EQ(trades[0].getPrice(), 155.0);
    EXPECT_EQ(trades[0].getQuantity(), 50);

    // Verify the BBO in the orderbook
    bestBid = engine.getOrderBookForRead(instrument)->getBestBid();
    ASSERT_NE(bestBid, nullptr);
    EXPECT_DOUBLE_EQ(bestBid->getPrice(), 150.0);
    EXPECT_EQ(bestBid->getQuantity(), 100);
    
    Order *bestAsk = engine.getOrderBookForRead(instrument)->getBestAsk();
    ASSERT_NE(bestAsk, nullptr);
    EXPECT_DOUBLE_EQ(bestAsk->getPrice(), 152.0);
    EXPECT_EQ(bestAsk->getQuantity(), 70);

    engine.getOrderBookForRead(instrument)->printOrderBook();
}

TEST(MatchingEngineTest, ModifyOrder)
{
    IDGenerator::getInstance().reset();
    MatchingEngine engine;
    std::string instrument = "AAPL";
    engine.addOrderBook(instrument);

    unsigned int buyOrderId = IDGenerator::getInstance().getNextOrderID();
    Order *buyOrder = new Order(buyOrderId, instrument, 150.0, 100, true, OrderType::LIMIT);
    engine.processNewOrder(buyOrder);

    double newPrice = 155.0;
    int newQuantity = 150;
    engine.modifyOrder(buyOrderId, instrument, newPrice, newQuantity);

    Order *bestBid = engine.getOrderBookForRead(instrument)->getBestBid();
    ASSERT_NE(bestBid, nullptr);
    EXPECT_DOUBLE_EQ(bestBid->getPrice(), newPrice);
    EXPECT_EQ(bestBid->getQuantity(), newQuantity);
}

TEST(MatchingEngineTest, CancelOrder)
{
    IDGenerator::getInstance().reset();
    MatchingEngine engine;
    std::string instrument = "AAPL";
    engine.addOrderBook(instrument);

    unsigned int sellOrderId = IDGenerator::getInstance().getNextOrderID();
    Order *sellOrder = new Order(sellOrderId, instrument, 160.0, 200, false, OrderType::LIMIT);
    engine.processNewOrder(sellOrder);

    engine.cancelOrder(sellOrderId, instrument);

    bool hasOrder = engine.hasOrder(instrument, sellOrderId);
    EXPECT_EQ(hasOrder, false);

    Order *bestAsk = engine.getOrderBookForRead(instrument)->getBestAsk();
    EXPECT_EQ(bestAsk, nullptr);
}

TEST(MatchingEngineTest, ModifyOrderCausingPriceCrossed)
{
    IDGenerator::getInstance().reset();
    MatchingEngine engine;
    std::string instrument = "AAPL";
    engine.addOrderBook(instrument);

    unsigned int sellOrderId = IDGenerator::getInstance().getNextOrderID();
    Order *sellOrder = new Order(sellOrderId, instrument, 155.0, 50, false, OrderType::LIMIT);
    engine.processNewOrder(sellOrder);

    unsigned int buyOrderId = IDGenerator::getInstance().getNextOrderID();
    Order *buyOrder = new Order(buyOrderId, instrument, 150.0, 100, true, OrderType::LIMIT);
    engine.processNewOrder(buyOrder);

    engine.modifyOrder(buyOrderId, instrument, 153.0, buyOrder->getQuantity());

    // Verify trade records and remaining orders added to the orderbook
    std::vector<Trade> trades = engine.getTrades();
    ASSERT_EQ(trades.size(), 0);

    const OrderBook *orderBook = engine.getOrderBookForRead(instrument);
    orderBook->printOrderBook();
}
