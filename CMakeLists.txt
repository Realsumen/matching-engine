cmake_minimum_required(VERSION 3.10)

option(DEBUG "Debug Mode" OFF)

if(DEBUG)
    message(STATUS "Debug Mode enabled")
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    endif()
endif()

project(MatchingEngineSystem VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(lib/spdlog)
add_subdirectory(lib/GTest)
add_subdirectory(lib/libuv)

include_directories(lib/rapidjson/include)

add_library(config_headers INTERFACE)
target_include_directories(config_headers INTERFACE include/config)

# Add utility files
file(GLOB UTILITY_SOURCES "utility/src/*.cpp" "utility/include/*.hpp")
add_library(utility_lib ${UTILITY_SOURCES})
target_include_directories(utility_lib PUBLIC utility/include)
target_link_libraries(utility_lib PUBLIC spdlog::spdlog config_headers libuv::libuv)

# Add gateway files
file(GLOB GATEWAY_SOURCES "gateway/src/*.cpp" "gateway/include/*.hpp")
add_library(gateway_lib ${GATEWAY_SOURCES})
target_include_directories(gateway_lib PUBLIC gateway/include lib/rapidjson/include)
target_link_libraries(gateway_lib PUBLIC utility_lib config_headers)

# Add matching engine files
file(GLOB MATCHING_ENGINE_SOURCES "matching_engine/src/*.cpp")
add_library(matching_engine_lib ${MATCHING_ENGINE_SOURCES})
target_include_directories(matching_engine_lib PUBLIC matching_engine/include utility/include gateway/include)
target_link_libraries(matching_engine_lib PRIVATE gateway_lib utility_lib spdlog::spdlog config_headers)

# Integrating system
file(GLOB SYSTEM_SOURCES "src/*.cpp")
list(REMOVE_ITEM SYSTEM_SOURCES "src/main.cpp")
add_library(system_lib ${SYSTEM_SOURCES})
target_include_directories(system_lib PUBLIC src/include matching_engine/include utility/include gateway/include)
target_link_libraries(system_lib PRIVATE gateway_lib utility_lib spdlog::spdlog config_headers)

# Main executable
add_executable(matching_engine_system src/main.cpp)
target_link_libraries(matching_engine_system PRIVATE system_lib matching_engine_lib gateway_lib spdlog::spdlog config_headers)

# Set output directories for main executable
set_target_properties(matching_engine_system PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Add custom flags for main executable
target_compile_options(matching_engine_system PRIVATE -Wall -Wextra -Werror)

option(USE_TSAN "Enable ThreadSanitizer" OFF)

if(USE_TSAN)
    message(STATUS "ThreadSanitizer enabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fno-omit-frame-pointer -g")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=thread")
endif()

option(USE_ASAN "Enable AddressSanitizer" OFF)

if(USE_ASAN)
    message(STATUS "AddressSanitizer enabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
endif()

# Add test executable
file(GLOB TEST_SOURCES "tests/*.cpp" "tests/*/*.cpp")
add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests
    matching_engine_lib
    gateway_lib
    utility_lib
    gtest_main
    config_headers
    # GTest::Main # A main function is already defined in Main.cpp
)

# Add custom flags for tests (without -Werror to prevent build failure on warnings)
target_compile_options(tests PRIVATE -Wall -Wextra)

# Enable testing
enable_testing()
add_test(NAME MatchingSystemTest COMMAND tests)

