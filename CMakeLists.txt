# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Add Debug Option
option(DEBUG "Debug Mode" OFF)

if(DEBUG)
    message(STATUS "Debug Mode enabled")
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    endif()
endif()

# Project name and version
project(MatchingEngine VERSION 0.1)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find required packages
find_package(spdlog REQUIRED)
find_package(GTest REQUIRED)

# Add utility files
file(GLOB UTILITY_SOURCES "utility/src/*.cpp" "utility/include/*.hpp")
add_library(utility_lib ${UTILITY_SOURCES})
target_include_directories(utility_lib PUBLIC utility/include)

# Add source files excluding main.cpp
file(GLOB MAIN_SOURCES "src/*.cpp")
list(REMOVE_ITEM MAIN_SOURCES "src/main.cpp")  # Remove the executable file to prevent repeating include
add_library(matching_engine_lib ${MAIN_SOURCES})
target_include_directories(matching_engine_lib PUBLIC include utility/include)
target_link_libraries(matching_engine_lib PRIVATE utility_lib spdlog::spdlog)

# Main executable
add_executable(matching_engine src/main.cpp)
target_link_libraries(matching_engine PRIVATE matching_engine_lib spdlog::spdlog)

# Set output directories for main executable
set_target_properties(matching_engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Add custom flags for main executable
target_compile_options(matching_engine PRIVATE -Wall -Wextra -Werror)

# Addx ASan Option
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
    utility_lib
    GTest::GTest
    # GTest::Main # A main function is already defined in Main.cpp
    spdlog::spdlog
)

# Add custom flags for tests (without -Werror to prevent build failure on warnings)
target_compile_options(tests PRIVATE -Wall -Wextra)

# Enable testing
enable_testing()
add_test(NAME MatchingEngineTest COMMAND tests)

