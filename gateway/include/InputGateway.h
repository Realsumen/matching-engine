#ifndef INPUT_GATEWAY_H
#define INPUT_GATEWAY_H

#include <uv.h>
#include <MessageQueue.h>
#include "Message.hpp"

class InputGateway
{
public:
    InputGateway(MessageQueue& messageQueue);
    ~InputGateway();

    void start(const char* ip, int port);
    void stop();

private:
    static void onnewConnection();
    static void onAllocBuffer();
    static void onRead(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf);

    uv_loop_t* loop_;
    uv_tcp_t server_;
    MessageQueue& messageQueue_;
};


#endif // INPUT_GATEWAY_H