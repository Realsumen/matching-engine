#ifndef GATEWAY_H
#define GATEWAY_H
#include <string>

class Gateway
{
public:
    virtual ~Gateway() = default;

    virtual void start(const std::string& ip, int port) = 0;
    virtual void stop() = 0;

    virtual void receive(const std::string& data, unsigned int client_id) = 0;
    virtual void send(const std::string& data) = 0;

};

#endif // GATEWAY_H
