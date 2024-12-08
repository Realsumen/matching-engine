#include "SystemLauncher.h"
#include "config.hpp"

int main() // NOLINT(*-use-trailing-return-type)
{
    const std::string address = mainConfig::ADDRESS;
    constexpr int port = mainConfig::PORT;

    SystemLauncher launcher(address, port);
    launcher.run();

    return 0;
}