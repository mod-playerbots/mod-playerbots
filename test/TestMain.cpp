/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>

/**
 * @brief Main entry point for mod-playerbots unit tests
 *
 * This test suite is designed to run independently of the AzerothCore server,
 * testing isolated components through mocks and interfaces.
 */
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
