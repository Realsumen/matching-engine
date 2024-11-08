#ifndef TRADE_H
#define TRADE_H

#include <string>
#include <chrono>

enum class TradeStatus
{
    SUCCESS,
    FAILED,
    PARTIALLY_FILLED,
    UNDEFINED
};

class Trade
{
public:
    Trade(int trade_id, int buy_order_id, int sell_order_id, 
        const std::string asset, double price, int quantity);

    const std::string &getAsset() const;
    unsigned int getTradeId() const;
    unsigned int getBuyOrderId() const;
    unsigned int getSellOrderId() const;
    double getPrice() const;
    double getTradeValue() const;
    int getQuantity() const;
    TradeStatus getBuyOrderStatus() const;
    TradeStatus getSellOrderStatus() const;
    std::chrono::time_point<std::chrono::high_resolution_clock> getTimestamp() const;

    void setBuyOrderStatus(TradeStatus status); // 设置买单状态
    void setSellOrderStatus(TradeStatus status); // 设置卖单状态
    std::string toString() const;

private:
    unsigned int trade_id;
    unsigned int buy_order_id;
    unsigned int sell_order_id;
    std::string asset;
    double price;
    int quantity;
    std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;
    TradeStatus buyOrderStatus;
    TradeStatus sellOrderStatus;
};

#endif