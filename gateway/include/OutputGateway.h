// OutputGateway.h

#ifndef OUTPUT_GATEWAY_H
#define OUTPUT_GATEWAY_H

#include <uv.h>
#include "Trade.h"

class DataGenerator;
class FrontEnd;

class OutputGateway {
public:
    OutputGateway();
    ~OutputGateway();

    void start(const char* ip, int port);
    void stop();

    void sendTrade(const Trade& trade);
    void sendOrderConfirmation(const std::string& orderId);

private:
    static void onWrite(uv_write_t* req, int status);

    uv_loop_t* loop_;
    uv_tcp_t client_;
    uv_connect_t connect_req_;
};

#endif // OUTPUT_GATEWAY_H
