#ifndef TEST_HPP
#define TEST_HPP

namespace TEST_Config {

    namespace OrderManager {
        constexpr auto INSTRUMENT = "AAPL";
        constexpr unsigned int PORT = 7001;
        constexpr float PRICE = 160.0;
    }

    namespace Integration {
        constexpr auto ADDRESS = "127.0.0.1";
        constexpr auto INSTRUMENT = "AAPL";
        constexpr unsigned int PORT = 7001;
    }

    namespace TCP {
        constexpr auto ADDRESS = "127.0.0.1";
        constexpr unsigned int PORT = 7001;
    }

}

#endif // TEST_HPP