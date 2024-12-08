#ifndef CONFIG_H
#define CONFIG_H

namespace mainConfig {
    constexpr auto ADDRESS = "127.0.0.1";
    constexpr unsigned int PORT = 7001;
}

namespace systemLauncher {
    // Logger Configuration
    constexpr auto LOGGER_NAME = "SystemLauncher";
    constexpr auto LOGGER_PATTERN = "%v"; // 仅显示日志消息

    // Command Names
    constexpr char STOP_COMMAND[] = "stop";
    constexpr char HELP_COMMAND[] = "help";
    constexpr char CREATE_ORDERBOOK_COMMAND[] = "create_orderbook";

    // Command Messages
    constexpr char CMD_HELP_MESSAGE[] =
        "Available commands:\n"
        "1. stop               - Stop the system.\n"
        "2. help               - Display this help message.\n"
        "3. create_orderbook   - Create a new order book. You will be prompted to enter an instrument name.\n\n"
        "Usage:\n"
        "Type the command name and press Enter.";

    constexpr char CMD_ENTER_COMMAND[] = "Enter command (type 'help' for a list of commands): ";
    constexpr char CMD_ENTER_INSTRUMENT[] = "Enter new Instrument name: ";
    constexpr char CMD_ORDERBOOK_CREATED[] = "OrderBook for instrument '{}' created successfully.";
    constexpr char CMD_ORDERBOOK_EXISTS[] = "OrderBook for instrument '{}' already exists.";
    constexpr char CMD_UNKNOWN_COMMAND[] = "Unknown command: {}";
    constexpr char CMD_EMPTY_INSTRUMENT[] = "Instrument name cannot be empty!";

    // Log Messages
    constexpr char LOG_EVENT_LOOP_STARTED[] = "Event loop started.";
    constexpr char LOG_INPUT_EVENT_LOOP_STARTED[] = "Input Event loop started...";
    constexpr char LOG_EVENT_LOOP_ERROR[] = "Event loop error occurred.";
    constexpr char LOG_EVENT_LOOP_STOPPED[] = "Event loop and manager stopped";
    constexpr char LOG_STOP_SIGNAL_RECEIVED[] = "Receive stop signal, stopping service...";
    constexpr char LOG_ASYNC_STOP_CLOSED[] = "async_stop_handle closed.";
    constexpr char LOG_INIT_ASYNC_FAILED[] = "Initialize async failed: {}";
    constexpr char LOG_COMMAND_RECEIVED[] = "Command '{}' received.";

    constexpr auto DEFAULT_ORDERBOOK_INSTRUMENT = "AAPL";
}


#endif // CONFIG_H