#include <iostream>
#include "TCPGateway.h"
#include "ProtocolParser.h"
#include <spdlog/fmt/ostr.h>

#include "IDGenerator.hpp"
#include "Logger.hpp"

template<>
struct fmt::formatter<uv_handle_type> {
    static constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const uv_handle_type& type, FormatContext& ctx) const -> decltype(ctx.out()) {
        const char* type_string = nullptr;
        switch (type) {
            case UV_UNKNOWN_HANDLE: type_string = "UV_UNKNOWN_HANDLE"; break;
            case UV_ASYNC: type_string = "UV_ASYNC"; break;
            case UV_CHECK: type_string = "UV_CHECK"; break;
            case UV_FS_EVENT: type_string = "UV_FS_EVENT"; break;
            case UV_FS_POLL: type_string = "UV_FS_POLL"; break;
            case UV_HANDLE: type_string = "UV_HANDLE"; break;
            case UV_IDLE: type_string = "UV_IDLE"; break;
            case UV_NAMED_PIPE: type_string = "UV_NAMED_PIPE"; break;
            case UV_POLL: type_string = "UV_POLL"; break;
            case UV_PREPARE: type_string = "UV_PREPARE"; break;
            case UV_PROCESS: type_string = "UV_PROCESS"; break;
            case UV_STREAM: type_string = "UV_STREAM"; break;
            case UV_TCP: type_string = "UV_TCP"; break;
            case UV_TIMER: type_string = "UV_TIMER"; break;
            case UV_TTY: type_string = "UV_TTY"; break;
            case UV_UDP: type_string = "UV_UDP"; break;
            case UV_SIGNAL: type_string = "UV_SIGNAL"; break;
            case UV_FILE: type_string = "UV_FILE"; break;
            default: type_string = "UNKNOWN_TYPE"; break;
        }
        return fmt::format_to(ctx.out(), "{}", type_string);
    }
};

TCPGateway::TCPGateway(uv_loop_t* loop, MessageQueue& messageQueue)
    : loop_(loop), messageQueue_(messageQueue), server_(nullptr), logger_(Logger::getLogger("TCPGateway")) {}

TCPGateway::~TCPGateway()
{
    // TCPGateway::stop();
    // instance_ = nullptr;
}


void TCPGateway::onAsyncCallback(uv_async_t *handle) {
    auto* gateway = static_cast<TCPGateway*>(handle->data);
    std::queue<OutgoingMessage> messagesToSend;

    {
        std::lock_guard<std::mutex> lock(gateway->outgoing_mutex_);
        std::swap(messagesToSend, gateway->outgoing_queue_);
    }

    while (!messagesToSend.empty()) {
        OutgoingMessage msg = std::move(messagesToSend.front());
        messagesToSend.pop();

        uv_stream_t* client = nullptr;
        {
            std::lock_guard<std::mutex> lock(gateway->client_map_mutex_);
            auto iter = gateway->client_map_.find(msg.client_id);
            if (iter != gateway->client_map_.end()) {
                client = iter->second;
            }
        }

        if (client != nullptr) {
            auto* write_req = new uv_write_t;
            auto* write_data = new std::string(msg.data);
            uv_buf_t write_buf = uv_buf_init(const_cast<char*>(write_data->c_str()), write_data->size());

            write_req->data = write_data;

            const int write_ret = uv_write(write_req, client, &write_buf, 1, [](uv_write_t* req, int status) {
                if (status < 0) {
                    std::cerr << "Write error: " << uv_strerror(status) << '\n';
                } else {
                    //
                }
                delete static_cast<std::string*>(req->data);
                delete req;
            });

            if (write_ret < 0) {
                gateway->logger_->error("uv_write error: {}", uv_strerror(write_ret));
                delete write_data;
                delete write_req;
            }
        } else {
            gateway->logger_->warn("Client ID {} not found. Unable to send message.", msg.client_id);
        }
    }
}

void TCPGateway::start(const std::string &ip, const int port)
{
    server_ = new uv_tcp_t;
    uv_tcp_init(loop_, server_);

    auto* serverData = new HandleData{this, true, 0};
    server_->data = serverData;

    int addr_ret = uv_ip4_addr(ip.c_str(), port, &addr_);
    if (addr_ret != 0) {
        logger_->error("Invalid IP address or port.");
        delete server_;
        server_ = nullptr;
        return;
    }

    int bind_ret = uv_tcp_bind(server_, reinterpret_cast<const sockaddr *>(&addr_), 0);
    if (bind_ret != 0) {
        logger_->error("Bind error: {}", uv_strerror(bind_ret));
        delete server_;
        server_ = nullptr;
        return;
    }

    // Start listening
    const int listen_ret = uv_listen(reinterpret_cast<uv_stream_t *>(server_), 128,
        [](uv_stream_t* client, const int status)
    {
        if (status < 0) {
            Logger::getLogger("TCPGateway")->error("Listen error: {}", uv_strerror(status));
            return;
        }

        auto* connection = new uv_tcp_t;
        if (uv_tcp_init(client->loop, connection) != 0) {
            Logger::getLogger("TCPGateway")->error("Failed to initialize connection.");
            delete connection;
            return;
        }

        int accept_ret = uv_accept(client, reinterpret_cast<uv_stream_t *>(connection));
        if (accept_ret == 0) 
        {
            auto* gateway = static_cast<TCPGateway*>(client->data);
            unsigned int client_id = IDGenerator::getInstance().getNextClientID();

            {
                std::lock_guard<std::mutex> lock(gateway->client_map_mutex_);
                gateway->client_map_[client_id] = reinterpret_cast<uv_stream_t*>(connection);
            }

            connection->data = new HandleData{gateway, false, client_id};

            if (uv_read_start(reinterpret_cast<uv_stream_t *>(connection), onAllocBuffer, onRead) != 0) {
                Logger::getLogger("TCPGateway")->error("Failed to start reading.");
                uv_close(reinterpret_cast<uv_handle_t *>(connection), onClientClosed);
            }

            gateway->logger_->info("New client connected with client_id: {}", client_id);
        }
        else
        {
            Logger::getLogger("TCPGateway")->error("Accept error: {}", uv_strerror(accept_ret));
            uv_close(reinterpret_cast<uv_handle_t *>(connection), onClientClosed);
        }

    }
    );
    if (listen_ret != 0) {
        logger_->error("Listen failed: {}", uv_strerror(listen_ret));
        uv_close(reinterpret_cast<uv_handle_t *>(server_), [](uv_handle_t* handle) {
            delete reinterpret_cast<uv_tcp_t*>(handle);
        });
        server_ = nullptr;
    }

    uv_async_init(loop_, &async_handle_, onAsyncCallback);
    async_handle_.data = this;
}

void TCPGateway::stop()
{
    if (server_ != nullptr) {
        uv_close(reinterpret_cast<uv_handle_t *>(server_), onClientClosed);
        server_ = nullptr;
    }

    {
        std::lock_guard<std::mutex> lock(client_map_mutex_);
        for (auto& [client_id, client] : client_map_) {
            auto* handle = reinterpret_cast<uv_handle_t*>(client);
            if (handle == nullptr) {
                logger_->warn("Invalid client handle: nullptr");
                continue;
            }

            if (handle->type != UV_TCP) {
                logger_->warn("Client handle type mismatch: {}", handle->type);
                continue;
            }

            if (uv_is_closing(handle) == 0) {
                logger_->info("Closing client handle: {}", static_cast<void*>(handle));
                uv_close(handle, onClientClosed);
            } else {
                logger_->info("Client handle is already closing: {}", static_cast<void*>(handle));
            }
        }
        client_map_.clear();
    }
    uv_close(reinterpret_cast<uv_handle_t*>(&async_handle_), nullptr);
    uv_run(loop_, UV_RUN_ONCE);
}

void TCPGateway::onRead(uv_stream_t *client, ssize_t bytesRead, const uv_buf_t *buf)
{
    auto* handleData = static_cast<HandleData*>(client->data);
    if (handleData == nullptr) {
        uv_close(reinterpret_cast<uv_handle_t *>(client), onClientClosed);
        delete[] buf->base;
        return;
    }

    auto* gateway = handleData->gateway;
    unsigned int client_id = handleData->client_id;

    if (bytesRead > 0)
    {
        const std::string data(buf->base, bytesRead);
        gateway->receive(data, client_id);
    } else if (bytesRead < 0) {
        if (bytesRead != UV_EOF) {
            gateway->logger_->error("Read error: {}", uv_err_name(static_cast<int>(bytesRead)));
        } else {
            gateway->logger_->info("Client {} disconnected.", client_id);
        }
        uv_close(reinterpret_cast<uv_handle_t *>(client), onClientClosed);
    }
    delete[] buf->base;
}

void TCPGateway::receive(const std::string& data, const unsigned int client_id)
{   
    Message message = ProtocolParser::parse(data, "TCP");
    message.client_id = client_id;
    messageQueue_.push(std::move(message));
}

void TCPGateway::send(const std::string &data) {
}

void TCPGateway::onAllocBuffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = new char[suggested_size];
    buf->len = suggested_size;
}

void TCPGateway::onWrite(uv_write_t *req, int status)
{
    if (status < 0) {
        std::cerr << "Write failed: " << uv_strerror(status) << '\n';
    }
    delete req;
}

void TCPGateway::onClientClosed(uv_handle_t* handle)
{
    auto* handleData = static_cast<HandleData*>(handle->data);
    if (!handleData) {
        delete reinterpret_cast<uv_tcp_t*>(handle);
        return;
    }
    if (handleData->isServer) {
        // 这是server handle
        handleData->gateway->logger_->info("Server handle closed.");
        delete reinterpret_cast<uv_tcp_t*>(handle);
        handleData->gateway->server_ = nullptr;
    }
    else {
        unsigned int client_id = handleData->client_id;

        {
            std::lock_guard<std::mutex> map_lock(handleData->gateway->client_map_mutex_);
            handleData->gateway->client_map_.erase(client_id);
        }

        handleData->gateway->logger_->info("Client handle closed: {} type: {}",
                                           static_cast<void*>(handle),
                                           handle->type);

        delete reinterpret_cast<uv_tcp_t*>(handle);
    }

    delete handleData;

}

void TCPGateway::queueMessageToSend(unsigned int client_id, const std::string &data) {
    OutgoingMessage msg;
    msg.client_id = client_id;
    msg.data = data;

    {
        std::lock_guard<std::mutex> lock(outgoing_mutex_);
        outgoing_queue_.push(std::move(msg));
    }

    uv_async_send(&async_handle_);
}
