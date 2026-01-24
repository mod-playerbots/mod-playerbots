/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TRIGGER_TEST_FIXTURE_H
#define _PLAYERBOT_TRIGGER_TEST_FIXTURE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "MockPlayerbotAI.h"
#include "MockPlayerbotAIConfig.h"

/**
 * @brief Base test fixture for testing Trigger classes
 *
 * This fixture provides common setup for testing trigger conditions
 * without requiring a running game server.
 *
 * Usage:
 * @code
 * class MyTriggerTest : public TriggerTestFixture
 * {
 * protected:
 *     void SetUp() override
 *     {
 *         TriggerTestFixture::SetUp();
 *         // Additional setup
 *     }
 * };
 *
 * TEST_F(MyTriggerTest, ShouldActivateWhenConditionMet)
 * {
 *     // Arrange
 *     EXPECT_CALL(mockConfig, GetCriticalHealth()).WillOnce(Return(20));
 *
 *     // Act & Assert
 *     // ... test trigger activation
 * }
 * @endcode
 */
class TriggerTestFixture : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mockConfig.SetupDefaults();
    }

    void TearDown() override
    {
        // Cleanup if needed
    }

    MockConfigProvider mockConfig;
    MockPlayerbotAI mockAI;
};

/**
 * @brief Test fixture for testing Value classes
 *
 * Similar to TriggerTestFixture but focused on Value calculations.
 */
class ValueTestFixture : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mockConfig.SetupDefaults();
    }

    void TearDown() override
    {
        // Cleanup if needed
    }

    MockConfigProvider mockConfig;
    MockPlayerbotAI mockAI;
};

/**
 * @brief Test fixture for testing Action classes
 *
 * Provides mocks needed for testing bot actions.
 */
class ActionTestFixture : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mockConfig.SetupDefaults();
    }

    void TearDown() override
    {
        // Cleanup if needed
    }

    MockConfigProvider mockConfig;
    MockPlayerbotAI mockAI;
};

#endif
