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
# IMPORTANT: fixtures must come FIRST to provide test stubs for AzerothCore headers
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/fixtures
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Bot
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Bot/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Bot/Engine
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Bot/Interface
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Bot/Service
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Bot/Handler
)

# Test sources
set(TEST_SOURCES
    TestMain.cpp

    # Bot Service tests
    unit/Bot/Service/ConfigProviderTest.cpp
    unit/Bot/Service/RoleServiceTest.cpp

    # Bot Core tests
    unit/Bot/Core/ManagerRegistryTest.cpp

    # Utility logic tests
    unit/HealthThresholdTest.cpp

    # AI Base pattern tests
    unit/Ai/Trigger/TriggerLogicTest.cpp
    unit/Ai/Value/ValueLogicTest.cpp
    unit/Ai/Action/ActionLogicTest.cpp

    # AI Engine tests
    unit/Ai/Engine/StrategyLogicTest.cpp
    unit/Ai/Engine/EngineTickTest.cpp
    unit/Ai/Engine/MultiplierTest.cpp
    unit/Ai/Engine/StateTransitionTest.cpp

    # AI Combat tests
    unit/Ai/Combat/TargetSelectionTest.cpp
    unit/Ai/Combat/SpellRotationTest.cpp
    unit/Ai/Combat/GroupCoordinationTest.cpp
    unit/Ai/Combat/BuffManagementTest.cpp
    unit/Ai/Combat/CrowdControlTest.cpp

    # AI Movement tests
    unit/Ai/Movement/PositioningTest.cpp

    # AI Resource tests
    unit/Ai/Resource/ResourceManagementTest.cpp

    # AI Pet tests
    unit/Ai/Pet/PetManagementTest.cpp

    # AI Dungeon/Raid tests
    unit/Ai/Dungeon/DungeonTacticsTest.cpp

    # Integration tests
    unit/Ai/Integration/ActionChainTest.cpp
)

# Create test executable
add_executable(playerbots_tests ${TEST_SOURCES})

# Force include the test prelude header for all test files
target_compile_options(playerbots_tests PRIVATE
    -include "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/TestPrelude.h"
)

target_link_libraries(playerbots_tests
    GTest::gtest_main
    GTest::gmock
)

include(GoogleTest)
gtest_discover_tests(playerbots_tests)
