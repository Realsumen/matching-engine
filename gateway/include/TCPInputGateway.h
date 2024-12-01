#ifndef TCP_INPUT_GATEWAY_H
#define TCP_INPUT_GATEWAY_H

#include "InputGateway.h"
#include "MessageQueue.h"
#include <uv.h>

class TCPInputGateway : public InputGateway
{
public:
    explicit TCPInputGateway(uv_loop_t* loop, MessageQueue& messageQueue);
    ~TCPInputGateway() override;

    void start(const std::string& ip, int port) override;
    void stop() override;
    void receive(const std::string& data) override;

private:
    uv_loop_t* loop_;
    MessageQueue& messageQueue_;
    uv_tcp_t* server_;

    static void onAllocBuffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
    static void onRead(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf);
};

#endif // TCP_INPUT_GATEWAY_H
