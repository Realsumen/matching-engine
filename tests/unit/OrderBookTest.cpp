#include <gtest/gtest.h>
#include "OrderBook.h"

TEST(OrderBookTest, Initialization) {
    OrderBook book = OrderBook("TSLA");
    std::cout << "Successfuly creates an OrderBook instance" << std::endl;
    std::cout << "Print the orderbook." << std::endl;
    book.printOrderBook();
    std::cout << "Print the orderbook with specified price range." << std::endl;
    book.printOrderBook(1, 100);
    std::cout << "Print the orderbook with specified depth." << std::endl;
    book.printOrderBook(10);
    std::cout << "============================================================" << std::endl;

}

TEST(OrderBookTest, AddOrder) {

}