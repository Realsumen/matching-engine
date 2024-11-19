#include <gtest/gtest.h>

// Entry point for Google Test
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); // Initialize GoogleTest
    return RUN_ALL_TESTS();                 // Execute all the test cases
}
