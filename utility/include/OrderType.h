#ifndef ORDER_TYPE_H
#define ORDER_TYPE_H

enum class OrderType : std::uint8_t {
    LIMIT,
    MARKET,
    STOP
};

#endif // ORDER_TYPE_H