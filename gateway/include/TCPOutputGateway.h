#ifndef TCP_OUTPUT_GATEWAY_H
#define TCP_OUTPUT_GATEWAY_H

#include "OutputGateway.h"
#include <uv.h>

class TCPOutputGateway : public OutputGateway
{
public:
    explicit TCPOutputGateway(uv_loop_t* loop);
    ~TCPOutputGateway() override;

    void start(const std::string& ip, int port) override;
    void stop() override;
    void send(const std::string& data) override;

private:
    uv_loop_t* loop_;
    uv_tcp_t tcpHandle_;
    struct sockaddr_in addr_;
};

#endif // TCP_OUTPUT_GATEWAY_H
