#ifndef GATEWAY_H
#define GATEWAY_H

#include <string>

class Gateway
{
public:
    virtual ~Gateway() = default;

    virtual void start(const std::string& ip, int port) = 0;
    virtual void stop() = 0;
};

#endif // GATEWAY_H
