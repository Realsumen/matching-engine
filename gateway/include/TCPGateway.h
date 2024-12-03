#ifndef TCP_GATEWAY_H
#define TCP_GATEWAY_H

#include "Gateway.h"
#include "MessageQueue.h"
#include <uv.h>

class TCPGateway : public Gateway {
public:
    explicit TCPGateway(uv_loop_t* loop, MessageQueue& messageQueue);
    ~TCPGateway() override;

    void start(const std::string& ip, int port) override;
    void stop() override;
    void receive(const std::string& data) override;
    void send(const std::string& data) override;

private:
    uv_loop_t* loop_;
    MessageQueue& messageQueue_;
    uv_tcp_t* server_;
    sockaddr_in addr_{};
    std::vector<uv_stream_t*> clients_;
    std::mutex clients_mutex_;

    static void onAllocBuffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
    static void onRead(uv_stream_t* client, ssize_t bytesRead, const uv_buf_t* buf);
    static void onWrite(uv_write_t* req, int status);

    void handleClient(uv_stream_t* client, const std::string& data);
};

#endif // TCP_GATEWAY_H
