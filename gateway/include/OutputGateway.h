#ifndef OUTPUT_GATEWAY_H
#define OUTPUT_GATEWAY_H

#include "Gateway.h"
#include <string>

class OutputGateway : public Gateway
{
public:
    virtual ~OutputGateway() = default;

    // Dedicated to sending data
    virtual void send(const std::string& data) = 0;
};

#endif // OUTPUT_GATEWAY_H
