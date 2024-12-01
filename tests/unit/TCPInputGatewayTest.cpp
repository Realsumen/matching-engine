#include <gtest/gtest.h>
#include "TCPInputGateway.h"
#include "ProtocolParser.h"

class TCPInputGatewayTest;

struct ClientContext {
    TCPInputGatewayTest* test;
    int client_id;
};

class TCPInputGatewayTest : public ::testing::Test
{
protected:
    uv_loop_t* loop;
    MessageQueue messageQueue;
    std::unique_ptr<TCPInputGateway> gateway;
    std::atomic<int> completedConnections; 
    int totalExpectedConnections;
    uv_timer_t* stop_timer;

    void SetUp() override 
    {
        loop = uv_default_loop();
        gateway = std::make_unique<TCPInputGateway>(loop, messageQueue);
        completedConnections = 0;
        totalExpectedConnections = 0;
        stop_timer = new uv_timer_t;
        uv_timer_init(loop, stop_timer);
        stop_timer->data = this;
    }

    void TearDown() override 
    {
        if (loop->active_handles > 0) {
            uv_stop(loop);
        }
        uv_loop_close(loop);
    }

    void check_and_stop_loop() {
        if (completedConnections >= totalExpectedConnections) {
            // 设置一个延迟，例如 100 毫秒
            schedule_stop_loop(100);
        }
    }

    void schedule_stop_loop(unsigned int delay_ms) {
        uv_timer_start(stop_timer, [](uv_timer_t* handle) {
            TCPInputGatewayTest* test = static_cast<TCPInputGatewayTest*>(handle->data);
            std::cout << "Stopping event loop after delay." << std::endl;
            uv_stop(test->loop);
            uv_timer_stop(handle);
        }, delay_ms, 0); 
    }

    // Callback function that accepts ClientContext
    static void on_connect(uv_connect_t* req, int status)
    {
        ClientContext* context = static_cast<ClientContext*>(req->data);
        ASSERT_EQ(status, 0);

        std::string message;
        if(context->client_id == 0){
            message = R"({ "type": "ADD_ORDER", "instrument": "GOOG", "price": 200.0, "quantity": 50, "isBuy": false, "orderType": "MARKET" })";
        }
        else
        {
            message = R"({ "type": "ADD_ORDER", "instrument": "MSFT", "price": 300.0, "quantity": 30, "isBuy": true, "orderType": "LIMIT" })";
        }
        uv_buf_t buf = uv_buf_init(const_cast<char*>(message.c_str()), static_cast<unsigned int>(message.size()));
        
        uv_write_t* write_req = new uv_write_t;
        write_req->data = context; 

        uv_write(write_req, req->handle, &buf, 1, on_write);
    }

    static void on_write(uv_write_t* req, int status)
    {
        ClientContext* context = static_cast<ClientContext*>(req->data);
        ASSERT_EQ(status, 0);
        auto test = context->test;
        delete req; // release write request
        delete context; // release context
        test->completedConnections++;
        test->check_and_stop_loop();
    }
};

TEST_F(TCPInputGatewayTest, ValidMessageParsing)
{
    std::vector<std::pair<std::string, std::function<void(const Message&)>>> testCases = {
        {
            R"({
                "type": "ADD_ORDER",
                "instrument": "AAPL",
                "price": 150.5,
                "quantity": 100,
                "isBuy": true,
                "orderType": "LIMIT"
            })",
            [](const Message& message) {
                ASSERT_EQ(message.type, MessageType::ADD_ORDER);
                ASSERT_EQ(message.addOrderDetails->instrument, "AAPL");
                ASSERT_DOUBLE_EQ(message.addOrderDetails->price, 150.5);
                ASSERT_EQ(message.addOrderDetails->quantity, 100);
                ASSERT_TRUE(message.addOrderDetails->isBuy);
                ASSERT_EQ(message.addOrderDetails->type, OrderType::LIMIT);
            }
        },
        {
            R"({
                "type": "MODIFY_ORDER",
                "orderId": 12345,
                "instrument": "AAPL",
                "newPrice": 155.0,
                "newQuantity": 200
            })",
            [](const Message& message) {
                ASSERT_EQ(message.type, MessageType::MODIFY_ORDER);
                ASSERT_EQ(message.modifyDetails->orderId, 12345);
                ASSERT_EQ(message.modifyDetails->instrument, "AAPL");
                ASSERT_DOUBLE_EQ(message.modifyDetails->newPrice, 155.0);
                ASSERT_EQ(message.modifyDetails->newQuantity, 200);
            }
        },
        {
            R"({
                "type": "CANCEL_ORDER",
                "orderId": 54321,
                "instrument": "AAPL"
            })",
            [](const Message& message) {
                ASSERT_EQ(message.type, MessageType::CANCEL_ORDER);
                ASSERT_EQ(message.cancelDetails->orderId, 54321);
                ASSERT_EQ(message.cancelDetails->instrument, "AAPL");
            }
        }
    };

    for (const auto& [jsonMessage, validator] : testCases) {
        gateway->receive(jsonMessage);

        ASSERT_FALSE(messageQueue.empty());
        Message message;
        bool success = messageQueue.tryPop(message);
        ASSERT_TRUE(success);

        validator(message);
    }
}

TEST_F(TCPInputGatewayTest, InvalidMessageHandling)
{
    std::vector<std::string> invalidMessages = {
        R"({ "type": "UNKNOWN" })", 
        R"({ "type": "ADD_ORDER", "price": 150.5, "quantity": 100 })",
        R"({ "type": "MODIFY_ORDER", "orderId": "invalid_id", "newPrice": 155.0 })", 
        R"({ "type": "CANCEL_ORDER" })", // 缺少必要字段
    };

    for (const auto& jsonMessage : invalidMessages) {
        EXPECT_THROW({ gateway->receive(jsonMessage); }, std::invalid_argument);
    }
}

TEST_F(TCPInputGatewayTest, ConnectionTests)
{   
    gateway->start("127.0.0.1", 7001);

    constexpr int numInitialClients = 1;
    constexpr int numAdditionalClients = 5;
    constexpr int totalClients = numInitialClients + numAdditionalClients;

    std::vector<uv_tcp_t> clients(totalClients);
    std::vector<uv_connect_t*> connectRequests(totalClients);
    sockaddr_in dest;
    uv_ip4_addr("127.0.0.1", 7001, &dest);

    totalExpectedConnections = totalClients;

    for (int i = 0; i < totalClients; ++i) {
        uv_tcp_init(loop, &clients[i]);
        connectRequests[i] = new uv_connect_t;
        ClientContext* context = new ClientContext{ this, i };
        connectRequests[i]->data = context; 

        uv_tcp_connect(connectRequests[i], &clients[i], (const sockaddr*)&dest, on_connect);
    }
    
    std::cout << "Starting event loop" << std::endl;
    uv_run(loop, UV_RUN_DEFAULT);
    std::cout << "Event loop stopped" << std::endl;

     for (int i = 0; i < totalClients; ++i) {
        ASSERT_FALSE(messageQueue.empty());
        Message message;
        bool success = messageQueue.tryPop(message);
        ASSERT_TRUE(success);
        if(i ==0){
            ASSERT_EQ(message.type, MessageType::ADD_ORDER);
            ASSERT_EQ(message.addOrderDetails->instrument, "GOOG");
            ASSERT_DOUBLE_EQ(message.addOrderDetails->price, 200.0);
            ASSERT_EQ(message.addOrderDetails->quantity, 50);
            ASSERT_FALSE(message.addOrderDetails->isBuy);
            ASSERT_EQ(message.addOrderDetails->type, OrderType::MARKET);
        }
        else{
            ASSERT_EQ(message.type, MessageType::ADD_ORDER);
            ASSERT_EQ(message.addOrderDetails->instrument, "MSFT");
            ASSERT_DOUBLE_EQ(message.addOrderDetails->price, 300.0);
            ASSERT_EQ(message.addOrderDetails->quantity, 30);
            ASSERT_TRUE(message.addOrderDetails->isBuy);
            ASSERT_EQ(message.addOrderDetails->type, OrderType::LIMIT);
        }
    }
    std::cout << "All messages verified" << std::endl;

    gateway->stop();

    for(auto& client : clients){
        uv_close(reinterpret_cast<uv_handle_t*>(&client), nullptr);
    }

    uv_run(loop, UV_RUN_DEFAULT);
}


