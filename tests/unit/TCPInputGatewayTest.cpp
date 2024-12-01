#include <gtest/gtest.h>
#include "TCPInputGateway.h"
#include "ProtocolParser.h"

class TCPInputGatewayTest : public ::testing::Test
{
protected:
    uv_loop_t* loop;
    MessageQueue messageQueue;
    std::unique_ptr<TCPInputGateway> gateway;

    void SetUp() override 
    {
        loop = uv_default_loop();
        gateway = std::make_unique<TCPInputGateway>(loop, messageQueue);
    }

    void TearDown() override 
    {
        uv_loop_close(loop);
    }
};

TEST_F(TCPInputGatewayTest, AddOrderMessageParsing)
{   
    std::string jsonMessage = R"({
        "type": "ADD_ORDER",
        "instrument": "AAPL",
        "price": 150.5,
        "quantity": 100,
        "isBuy": true,
        "orderType": "LIMIT"
    })";

    gateway->receive(jsonMessage);

    ASSERT_FALSE(messageQueue.empty());
    Message message;
    bool success = messageQueue.tryPop(message);
    ASSERT_TRUE(success);

    
    ASSERT_EQ(message.type, MessageType::ADD_ORDER);
    ASSERT_EQ(message.addOrderDetails->instrument, "AAPL");
    ASSERT_DOUBLE_EQ(message.addOrderDetails->price, 150.5);
    ASSERT_EQ(message.addOrderDetails->quantity, 100);
    ASSERT_TRUE(message.addOrderDetails->isBuy);
    ASSERT_EQ(message.addOrderDetails->type, OrderType::LIMIT);
}

TEST_F(TCPInputGatewayTest, ModifyOrderMessageParsing)
{
    std::string jsonMessage = R"({
        "type": "MODIFY_ORDER",
        "orderId": 12345,
        "instrument": "AAPL",
        "newPrice": 155.0,
        "newQuantity": 200
    })";

    gateway->receive(jsonMessage);

    ASSERT_FALSE(messageQueue.empty());
    Message message;
    bool success = messageQueue.tryPop(message);
    ASSERT_TRUE(success);

    ASSERT_EQ(message.type, MessageType::MODIFY_ORDER);
    ASSERT_EQ(message.modifyDetails->orderId, 12345);
    ASSERT_EQ(message.modifyDetails->instrument, "AAPL");
    ASSERT_DOUBLE_EQ(message.modifyDetails->newPrice, 155.0);
    ASSERT_EQ(message.modifyDetails->newQuantity, 200);
}

TEST_F(TCPInputGatewayTest, CancelOrderMessageParsing)
{
    std::string jsonMessage = R"({
        "type": "CANCEL_ORDER",
        "orderId": 54321,
        "instrument": "AAPL"
    })";

    gateway->receive(jsonMessage);

    ASSERT_FALSE(messageQueue.empty());
    Message message;
    bool success = messageQueue.tryPop(message);
    ASSERT_TRUE(success);

    ASSERT_EQ(message.type, MessageType::CANCEL_ORDER);
    ASSERT_EQ(message.cancelDetails->orderId, 54321);
    ASSERT_EQ(message.cancelDetails->instrument, "AAPL");
}

TEST_F(TCPInputGatewayTest, InvalidJsonMessage)
{
    std::string invalidJsonMessage = R"({ "type": "UNKNOWN" })";

    EXPECT_THROW({ gateway->receive(invalidJsonMessage); }, std::invalid_argument);
}

TEST_F(TCPInputGatewayTest, MissingFieldsInMessage)
{
    std::string missingFieldsMessage = R"({
        "type": "ADD_ORDER",
        "price": 150.5,
        "quantity": 100
    })";

    EXPECT_THROW({ gateway->receive(missingFieldsMessage); }, std::invalid_argument);
}
