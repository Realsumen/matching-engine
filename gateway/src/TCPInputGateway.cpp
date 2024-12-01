#include <iostream>
#include "TCPInputGateway.h"
#include "ProtocolParser.h"

TCPInputGateway::TCPInputGateway(uv_loop_t* loop, MessageQueue& messageQueue)
    : loop_(loop), messageQueue_(messageQueue), server_(nullptr) {}

TCPInputGateway::~TCPInputGateway()
{
    stop();
}

void TCPInputGateway::start(const std::string &ip, int port)
{
    server_ = new uv_tcp_t;
    uv_tcp_init(loop_, server_);
    server_->data = this;

    sockaddr_in addr;
    int addr_ret = uv_ip4_addr(ip.c_str(), port, &addr);
    if (addr_ret != 0) {
        std::cerr << "Invalid IP address or port." << std::endl;
        delete server_;
        server_ = nullptr;
        return;
    }

    int bind_ret = uv_tcp_bind(server_, (const sockaddr*)&addr, 0);
    if (bind_ret != 0) {
        std::cerr << "Bind error: " << uv_strerror(addr_ret) << std::endl;
        delete server_;
        server_ = nullptr;
        return;
    }

    // Start lisening
    int listen_ret = uv_listen((uv_stream_t*)server_, 128, [](uv_stream_t* client, int status) 
    {
        if (status < 0) {
            std::cerr << "Listen error: " << uv_strerror(status) << std::endl;
            return;
        }

        uv_tcp_t* connection = new uv_tcp_t;
        if (uv_tcp_init(client->loop, connection) != 0) {
            std::cerr << "Failed to initialize connection." << std::endl;
            delete connection;
            return;
        }

        int accept_ret = uv_accept(client, (uv_stream_t*)connection);
        if (accept_ret == 0) 
        {
            TCPInputGateway* gateway = static_cast<TCPInputGateway*>(client->data);
            connection->data = gateway;

            if (uv_read_start((uv_stream_t*)connection, onAllocBuffer, onRead) != 0) {
                std::cerr << "Failed to start reading." << std::endl;
                uv_close((uv_handle_t*)connection, [](uv_handle_t* handle) {
                    delete reinterpret_cast<uv_tcp_t*>(handle);
                });
            }
        }
        else
        {
            std::cerr << "Accept error: " << uv_strerror(accept_ret) << std::endl;
            uv_close((uv_handle_t*)connection, [](uv_handle_t* handle) {
                delete reinterpret_cast<uv_tcp_t*>(handle);
            });
        }

    }
    );
    if (listen_ret != 0) {
        std::cerr << "Listen failed: " << uv_strerror(listen_ret) << std::endl;
        uv_close((uv_handle_t*)server_, [](uv_handle_t* handle) {
            delete reinterpret_cast<uv_tcp_t*>(handle);
        });
        server_ = nullptr;
    }
}

void TCPInputGateway::stop()
{
    if (server_) {
        uv_close((uv_handle_t*)server_, [](uv_handle_t* handle) {
            delete reinterpret_cast<uv_tcp_t*>(handle);
        });
        server_ = nullptr;
    }
}

void TCPInputGateway::receive(const std::string& data)
{   
    Message message = ProtocolParser::parse(data, "TCP");
    messageQueue_.push(std::move(message));
}

void TCPInputGateway::onAllocBuffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = new char[suggested_size];
    buf->len = suggested_size;
}

void TCPInputGateway::onRead(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    if (nread > 0)
    {
        TCPInputGateway* gateway = static_cast<TCPInputGateway*>(client->data);
        std::string data(buf->base, nread);
        gateway->receive(data);
    } else if (nread < 0) {
        if (nread != UV_EOF)
            std::cerr << "Read error " << uv_err_name(nread) << std::endl;
        uv_close((uv_handle_t*)client, [](uv_handle_t* handle) {
            delete reinterpret_cast<uv_tcp_t*>(handle);
        });
    }
    delete[] buf->base;
}