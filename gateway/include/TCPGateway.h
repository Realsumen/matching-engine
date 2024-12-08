#ifndef TCP_GATEWAY_H
#define TCP_GATEWAY_H

#include "Gateway.h"
#include "MessageQueue.h"
#include "Logger.hpp"
#include <uv.h>

class TCPGateway : public Gateway {
public:
    explicit TCPGateway(uv_loop_t* loop, MessageQueue& messageQueue);
    ~TCPGateway() override;

    void start(const std::string& ip, int port) override;
    void stop() override;
    void receive(const std::string& data, unsigned int client_id) override;
    void send(const std::string& data) override;
    void queueMessageToSend(unsigned int client_id, const std::string& data);

private:
    //
    struct HandleData {
        TCPGateway* gateway;
        bool isServer;
        unsigned int client_id;
    };

    // Data structure for the data to be sent.
    struct OutgoingMessage {
        unsigned int client_id;
        std::string data;
    };

    static void onAllocBuffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
    static void onAsyncCallback(uv_async_t* handle);
    static void onRead(uv_stream_t* client, ssize_t bytesRead, const uv_buf_t* buf);
    static void onWrite(uv_write_t* req, int status);
    static void onClientClosed(uv_handle_t *handle);

    uv_loop_t* loop_;
    MessageQueue& messageQueue_;
    uv_tcp_t* server_;
    sockaddr_in addr_{};
    std::shared_ptr<spdlog::logger> logger_;

    std::mutex client_map_mutex_;
    std::unordered_map<unsigned int, uv_stream_t*> client_map_;
    std::atomic<int> activeHandles_{0};

    std::queue<OutgoingMessage> outgoing_queue_;
    std::mutex outgoing_mutex_;
    uv_async_t async_handle_{};
};

#endif // TCP_GATEWAY_H
