#ifndef INPUT_GATEWAY_H
#define INPUT_GATEWAY_H

#include "Gateway.h"
#include <string>

class InputGateway : public Gateway
{
public:
    virtual ~InputGateway() = default;

    // Dedicated to receiving data
    virtual void receive(const std::string& data) = 0;
};

#endif // INPUT_GATEWAY_H