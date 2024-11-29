#include <iostream>
#include "Order.h"
#include "Trade.h"
#include <uv.h>
#include <iostream>

int main() {
    uv_loop_t *loop = uv_default_loop();
    std::cout << "Libuv version: " << uv_version_string() << std::endl;

    uv_timer_t timer;
    uv_timer_init(loop, &timer);
    uv_timer_start(&timer, [](uv_timer_t *handle) {
        std::cout << "Timer fired!" << std::endl;
        uv_stop(handle->loop);
    }, 1000, 0);

    uv_run(loop, UV_RUN_DEFAULT);
    uv_loop_close(loop);
    return 0;
}
