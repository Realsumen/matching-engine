# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(MatchingEngine VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add utility files
file(GLOB UTILITY_SOURCES "utility/src/*.cpp")

add_library(utility_lib ${UTILITY_SOURCES})

target_include_directories(utility_lib PUBLIC utility/include)

# Add source files
file(GLOB MAIN_SOURCES "src/*.cpp")

# Add executable target with source files
add_executable(matching_engine main.cpp ${MAIN_SOURCES})

target_include_directories(matching_engine PUBLIC include)

# Link utility library
target_link_libraries(matching_engine utility_lib)

# Set output directories
set_target_properties(matching_engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Add custom flags if necessary (example: warnings)
target_compile_options(matching_engine PRIVATE -Wall -Wextra -Werror)

# Add test executable
add_executable(test tests/test.cpp ${MAIN_SOURCES})

# Link utility library
target_link_libraries(test utility_lib)

# Set output directories for the test executable
set_target_properties(test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Add custom flags for test if necessary
target_compile_options(test PRIVATE -Wall -Wextra -Werror)
