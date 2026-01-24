# mod-playerbots test configuration
# This CMakeLists.txt is intended for standalone testing with Google Test

cmake_minimum_required(VERSION 3.16)
project(mod-playerbots-tests)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Bot
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Bot/Engine
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Bot/Interface
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Bot/Service
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${CMAKE_CURRENT_SOURCE_DIR}/fixtures
)

# Test sources
set(TEST_SOURCES
    TestMain.cpp
    # Add unit tests here as they are created
    # unit/Bot/Service/ConfigProviderTest.cpp
)

# Create test executable
add_executable(playerbots_tests ${TEST_SOURCES})

target_link_libraries(playerbots_tests
    GTest::gtest_main
    GTest::gmock
)

include(GoogleTest)
gtest_discover_tests(playerbots_tests)
