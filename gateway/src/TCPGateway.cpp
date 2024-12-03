#include <iostream>
#include "TCPGateway.h"
#include "ProtocolParser.h"

TCPGateway::TCPGateway(uv_loop_t* loop, MessageQueue& messageQueue)
    : loop_(loop), messageQueue_(messageQueue), server_(nullptr) {}

TCPGateway::~TCPGateway()
{
    TCPGateway::stop();
}

void TCPGateway::start(const std::string &ip, const int port)
{
    server_ = new uv_tcp_t;
    uv_tcp_init(loop_, server_);
    server_->data = this;

    int addr_ret = uv_ip4_addr(ip.c_str(), port, &addr_);
    if (addr_ret != 0) {
        std::cerr << "Invalid IP address or port." << '\n';
        delete server_;
        server_ = nullptr;
        return;
    }

    int bind_ret = uv_tcp_bind(server_, reinterpret_cast<const sockaddr *>(&addr_), 0);
    if (bind_ret != 0) {
        std::cerr << "Bind error: " << uv_strerror(addr_ret) << '\n';
        delete server_;
        server_ = nullptr;
        return;
    }

    // Start listening
    int listen_ret = uv_listen(reinterpret_cast<uv_stream_t *>(server_), 128, [](uv_stream_t* client, const int status)
    {
        if (status < 0) {
            std::cerr << "Listen error: " << uv_strerror(status) << '\n';
            return;
        }

        auto* connection = new uv_tcp_t;
        if (uv_tcp_init(client->loop, connection) != 0) {
            std::cerr << "Failed to initialize connection." << '\n';
            delete connection;
            return;
        }

        int accept_ret = uv_accept(client, reinterpret_cast<uv_stream_t *>(connection));
        if (accept_ret == 0) 
        {
            auto* gateway = static_cast<TCPGateway*>(client->data);
            connection->data = gateway;

            {
                std::lock_guard<std::mutex> lock(gateway->clients_mutex_);
                gateway->clients_.push_back(reinterpret_cast<uv_stream_t*>(connection));
            }


            if (uv_read_start(reinterpret_cast<uv_stream_t *>(connection), onAllocBuffer, onRead) != 0) {
                std::cerr << "Failed to start reading." << '\n';
                uv_close(reinterpret_cast<uv_handle_t *>(connection), [](uv_handle_t* handle) {
                    delete reinterpret_cast<uv_tcp_t*>(handle);
                });
            }
        }
        else
        {
            std::cerr << "Accept error: " << uv_strerror(accept_ret) << '\n';
            uv_close(reinterpret_cast<uv_handle_t *>(connection), [](uv_handle_t* handle) {
                delete reinterpret_cast<uv_tcp_t*>(handle);
            });
        }

    }
    );
    if (listen_ret != 0) {
        std::cerr << "Listen failed: " << uv_strerror(listen_ret) << '\n';
        uv_close(reinterpret_cast<uv_handle_t *>(server_), [](uv_handle_t* handle) {
            delete reinterpret_cast<uv_tcp_t*>(handle);
        });
        server_ = nullptr;
    }
}

void TCPGateway::stop()
{
    if (server_ != nullptr) {
        uv_close(reinterpret_cast<uv_handle_t *>(server_), [](uv_handle_t* handle) {
            delete reinterpret_cast<uv_tcp_t*>(handle);
        });
        server_ = nullptr;
    }

    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (auto* client : clients_) {
        uv_close(reinterpret_cast<uv_handle_t *>(client), [](uv_handle_t* handle) {
            delete reinterpret_cast<uv_tcp_t*>(handle);
        });
    }
    clients_.clear();
}

void TCPGateway::receive(const std::string& data)
{   
    Message message = ProtocolParser::parse(data, "TCP");
    messageQueue_.push(std::move(message));
}

void TCPGateway::send(const std::string &data)
{
    // TODO: implement send function
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (auto* client : clients_) {
        // 创建写请求
        auto* write_req = new uv_write_t;
        uv_buf_t buf = uv_buf_init(const_cast<char*>(data.c_str()), data.size());

        // 你可以在 write_req->data 中存储更多信息，例如要发送的数据等
        write_req->data = this;

        int write_ret = uv_write(write_req, client, &buf, 1, TCPGateway::onWrite);
        if (write_ret < 0) {
            std::cerr << "Write error: " << uv_strerror(write_ret) << '\n';
            delete write_req; // 立即释放写请求以避免内存泄漏
        }
    }
}

void TCPGateway::onAllocBuffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = new char[suggested_size];
    buf->len = suggested_size;
}

void TCPGateway::onRead(uv_stream_t *client, ssize_t bytesRead, const uv_buf_t *buf)
{
    if (bytesRead > 0)
    {
        auto* gateway = static_cast<TCPGateway*>(client->data);
        const std::string data(buf->base, bytesRead);
        gateway->receive(data);
    } else if (bytesRead < 0) {
        if (bytesRead != UV_EOF) {
            std::cerr << "Read error " << uv_err_name(static_cast<int>(bytesRead)) << '\n';
        }
        uv_close(reinterpret_cast<uv_handle_t *>(client), [](uv_handle_t* handle) {
            delete reinterpret_cast<uv_tcp_t*>(handle);
        });
    }
    delete[] buf->base;
}

void TCPGateway::onWrite(uv_write_t *req, int status)
{
    if (status < 0) {
        std::cerr << "Write failed: " << uv_strerror(status) << '\n';
    } else {
        std::cout << "Write successful." << '\n';
    }

    delete req;
}

void TCPGateway::handleClient(uv_stream_t *client, const std::string &data)
{
}
