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
    engine.createNewOrderBook(instrument);

    // Creates a limit buy order
    unsigned int buyOrderId = IDGenerator::getInstance().getNextOrderID();
    Order *buyOrder = Order::CreateLimitOrder(buyOrderId, instrument, 150.0, 100, true);
    std::vector<Trade> trades = engine.processNewOrder(buyOrder);

    EXPECT_TRUE(trades.empty());

    Order *bestBid = engine.getOrderBookForRead(instrument)->getBestBid();
    ASSERT_NE(bestBid, nullptr);
    EXPECT_DOUBLE_EQ(bestBid->getPrice(), 150.0);

    // Creats a limit sell order, which has price crossed with the best Bid
    unsigned int sellOrderId = IDGenerator::getInstance().getNextOrderID();
    Order *sellOrder = Order::CreateLimitOrder(sellOrderId, instrument, 150.0, 120, false);
    trades = engine.processNewOrder(sellOrder);

    // There should be a transaction
    ASSERT_EQ(trades.size(), 1);
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
    engine.createNewOrderBook(instrument);

    // Add Limit Order on Bid side
    unsigned int buyOrderId1 = IDGenerator::getInstance().getNextOrderID();
    Order *buyOrder1 = Order::CreateLimitOrder(buyOrderId1, instrument, 150.0, 100, true);
    engine.processNewOrder(buyOrder1);

    unsigned int buyOrderId2 = IDGenerator::getInstance().getNextOrderID();
    Order *buyOrder2 = Order::CreateLimitOrder(buyOrderId2, instrument, 155.0, 50, true);
    engine.processNewOrder(buyOrder2);

    Order *bestBid = engine.getOrderBookForRead(instrument)->getBestBid();
    ASSERT_NE(bestBid, nullptr);
    EXPECT_DOUBLE_EQ(bestBid->getPrice(), 155.0);
    EXPECT_EQ(bestBid->getQuantity(), 50);

    // Add Limit Order on Ask side
    unsigned int sellOrderId1 = IDGenerator::getInstance().getNextOrderID();
    Order *sellOrder1 = Order::CreateLimitOrder(sellOrderId1, instrument, 152.0, 120, false);
    engine.processNewOrder(sellOrder1);

    unsigned int sellOrderId2 = IDGenerator::getInstance().getNextOrderID();
    Order *sellOrder2 = Order::CreateLimitOrder(sellOrderId2, instrument, 160.0, 25, false);
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
    engine.createNewOrderBook(instrument);

    unsigned int buyOrderId = IDGenerator::getInstance().getNextOrderID();
    Order *buyOrder = Order::CreateLimitOrder(buyOrderId, instrument, 150.0, 100, true);
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
    engine.createNewOrderBook(instrument);

    unsigned int sellOrderId = IDGenerator::getInstance().getNextOrderID();
    Order *sellOrder = Order::CreateLimitOrder(sellOrderId, instrument, 160.0, 200, false);
    engine.processNewOrder(sellOrder);

    engine.cancelOrder(sellOrderId, instrument);

    bool hasOrder = engine.hasOrder(instrument, sellOrderId);
    EXPECT_EQ(hasOrder, false);

    Order *bestAsk = engine.getOrderBookForRead(instrument)->getBestAsk();
    EXPECT_EQ(bestAsk, nullptr);
}

TEST(MatchingEngineTest, MarketOrder)
{
    IDGenerator::getInstance().reset();
    MatchingEngine engine;
    std::string instrument = "AAPL";
    engine.createNewOrderBook(instrument);

    unsigned int limitBuyOrderId = IDGenerator::getInstance().getNextOrderID();
    Order *limitBuyOrder = Order::CreateLimitOrder(limitBuyOrderId, instrument, 150, 100, true);
    engine.processNewOrder(limitBuyOrder);

    unsigned int limitSellOrderId = IDGenerator::getInstance().getNextOrderID();
    Order *limitSellOrder = Order::CreateLimitOrder(limitSellOrderId, instrument, 155.0, 50, false);
    engine.processNewOrder(limitSellOrder);

    unsigned int marketBuyOrderId = IDGenerator::getInstance().getNextOrderID();
    Order *marketBuyOrder = Order::CreateMarketOrder(marketBuyOrderId, instrument, 60, true);
    std::vector<Trade> trades = engine.processNewOrder(marketBuyOrder);

    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].getBuyOrderId(), marketBuyOrderId);
    EXPECT_EQ(trades[0].getSellOrderId(), limitSellOrderId);
    EXPECT_DOUBLE_EQ(trades[0].getPrice(), 155.0);
    EXPECT_EQ(trades[0].getQuantity(), 50);

    unsigned int marketSellOrderId = IDGenerator::getInstance().getNextOrderID();
    Order *marketSellOrder = Order::CreateMarketOrder(marketSellOrderId, instrument, 80, false);
    trades = engine.processNewOrder(marketSellOrder);

    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].getBuyOrderId(), limitBuyOrderId);
    EXPECT_EQ(trades[0].getSellOrderId(), marketSellOrderId);
    EXPECT_DOUBLE_EQ(trades[0].getPrice(), 150.0);
    EXPECT_EQ(trades[0].getQuantity(), 80);

    Order *remainingLimitBuyOrder = engine.getOrderBookForRead(instrument)->getBestBid();
    ASSERT_NE(remainingLimitBuyOrder, nullptr);
    EXPECT_EQ(remainingLimitBuyOrder->getQuantity(), 20);

}
