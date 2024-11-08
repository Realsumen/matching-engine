#include <iostream>
#include "Order.h"
#include "Trade.h"

int main() {
    Order buyLimitOrder = Order::CreateLimitOrder(1, "AAPL", 150.0, 10, true);
    buyLimitOrder.displayOrderInfo();

    Order sellMarketOrder = Order::CreateMarketOrder(2, "AAPL", 3, false);
    buyLimitOrder.displayOrderInfo();

    Order stopBuyOrder = Order::CreateStopOrder(3, "AAPL", 155.0, 7, true);
    buyLimitOrder.displayOrderInfo();

    buyLimitOrder.setPrice(152.5);
    buyLimitOrder.setQuantity(12);
    std::cout << "After modifications:" << std::endl;
    buyLimitOrder.displayOrderInfo();

    std::cout << "Comparing orders:" << std::endl;
    if (buyLimitOrder == stopBuyOrder) {
        std::cout << "Orders are equal." << std::endl;
    } else if (buyLimitOrder < stopBuyOrder) {
        std::cout << "Limit order is smaller than stop order." << std::endl;
    } else {
        std::cout << "Limit order is greater than stop order." << std::endl;
    }

    Order buyOrder = Order::CreateLimitOrder(1, "AAPL", 150.0, 10, true);
    Order sellOrder = Order::CreateMarketOrder(2, "AAPL", 5, false);

    Trade trade(1, buyOrder.getId(), sellOrder.getId(), "AAPL", 150.0, 5);
    std::cout << "Trade Details: " << trade.toString() << std::endl;

    trade.setStatus(TradeStatus::PARTIALLY_FILLED);
    std::cout << "Trade Status after partial fill: " << trade.toString() << std::endl;

    return 0;
}
