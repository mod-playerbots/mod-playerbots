/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>
#include "mocks/MockPlayerbotAIConfig.h"

/**
 * @brief Tests for the MockConfigProvider
 *
 * These tests demonstrate how to use the mock configuration provider
 * for testing bot components that depend on configuration values.
 */
class ConfigProviderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mockConfig.SetupDefaults();
    }

    MockConfigProvider mockConfig;
};

TEST_F(ConfigProviderTest, DefaultDistanceValues)
{
    // Verify default distance values are set correctly
    EXPECT_FLOAT_EQ(75.0f, mockConfig.GetSightDistance());
    EXPECT_FLOAT_EQ(26.0f, mockConfig.GetSpellDistance());
    EXPECT_FLOAT_EQ(150.0f, mockConfig.GetReactDistance());
    EXPECT_FLOAT_EQ(1.5f, mockConfig.GetMeleeDistance());
    EXPECT_FLOAT_EQ(38.0f, mockConfig.GetHealDistance());
}

TEST_F(ConfigProviderTest, DefaultHealthThresholds)
{
    // Verify default health thresholds
    EXPECT_EQ(20u, mockConfig.GetCriticalHealth());
    EXPECT_EQ(45u, mockConfig.GetLowHealth());
    EXPECT_EQ(65u, mockConfig.GetMediumHealth());
    EXPECT_EQ(85u, mockConfig.GetAlmostFullHealth());
}

TEST_F(ConfigProviderTest, DefaultManaThresholds)
{
    // Verify default mana thresholds
    EXPECT_EQ(15u, mockConfig.GetLowMana());
    EXPECT_EQ(40u, mockConfig.GetMediumMana());
    EXPECT_EQ(80u, mockConfig.GetHighMana());
}

TEST_F(ConfigProviderTest, DefaultTimingValues)
{
    // Verify default timing values
    EXPECT_EQ(1500u, mockConfig.GetGlobalCooldown());
    EXPECT_EQ(100u, mockConfig.GetReactDelay());
    EXPECT_EQ(5000u, mockConfig.GetMaxWaitForMove());
}

TEST_F(ConfigProviderTest, DefaultFeatureFlags)
{
    // Verify default feature flags
    EXPECT_TRUE(mockConfig.IsEnabled());
    EXPECT_TRUE(mockConfig.IsFleeingEnabled());
    EXPECT_TRUE(mockConfig.IsAutoAvoidAoe());
    EXPECT_FALSE(mockConfig.IsDynamicReactDelay());
}

TEST_F(ConfigProviderTest, CanOverrideWithExpectCall)
{
    using ::testing::Return;

    // Override specific values for a test
    EXPECT_CALL(mockConfig, GetCriticalHealth()).WillOnce(Return(25u));
    EXPECT_CALL(mockConfig, GetLowHealth()).WillOnce(Return(50u));

    EXPECT_EQ(25u, mockConfig.GetCriticalHealth());
    EXPECT_EQ(50u, mockConfig.GetLowHealth());
}

TEST_F(ConfigProviderTest, CanMockLogging)
{
    using ::testing::Return;
    using ::testing::_;

    // Default returns false for HasLog
    EXPECT_FALSE(mockConfig.HasLog("test.log"));

    // Can override for specific files
    EXPECT_CALL(mockConfig, HasLog("debug.log")).WillOnce(Return(true));
    EXPECT_TRUE(mockConfig.HasLog("debug.log"));
}
