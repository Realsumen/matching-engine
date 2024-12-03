#ifndef TRADE_H
#define TRADE_H

#include <string>

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
    Trade(unsigned int trade_id, unsigned int buy_order_id, unsigned int sell_order_id,
         std::string  asset, double price, int quantity);

    [[nodiscard]] auto getAsset() const -> const std::string &;
    [[nodiscard]] auto getTradeId() const -> unsigned int;
    [[nodiscard]] auto getBuyOrderId() const -> unsigned int;
    [[nodiscard]] auto getSellOrderId() const -> unsigned int;
    [[nodiscard]] auto getPrice() const -> double;
    [[nodiscard]] auto getTradeValue() const -> double;
    [[nodiscard]] auto getQuantity() const -> int;
    [[nodiscard]] auto getBuyOrderStatus() const -> TradeStatus;
    [[nodiscard]] auto getSellOrderStatus() const -> TradeStatus;
    [[nodiscard]] auto getTimestamp() const -> std::chrono::system_clock::time_point;

    void setBuyOrderStatus(TradeStatus status);
    void setSellOrderStatus(TradeStatus status);
    [[nodiscard]] auto toString(const std::string& format = "default") const -> std::string;

private:
    unsigned int trade_id;
    unsigned int buy_order_id;
    unsigned int sell_order_id;
    std::string asset;
    double price;
    int quantity;
    std::chrono::system_clock::time_point timestamp;
    TradeStatus buyOrderStatus;
    TradeStatus sellOrderStatus;
};

#endif