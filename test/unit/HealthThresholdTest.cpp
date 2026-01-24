/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>
#include "mocks/MockPlayerbotAIConfig.h"

/**
 * @brief Tests for health threshold logic
 *
 * These tests validate the logic used by health triggers
 * without requiring actual game objects.
 */
class HealthThresholdTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mockConfig.SetupDefaults();
    }

    MockConfigProvider mockConfig;

    /**
     * @brief Check if health percentage is in critical range
     * @param healthPercent Current health as percentage (0-100)
     * @param threshold Critical health threshold
     * @return true if health is at or below critical threshold
     */
    bool IsCriticalHealth(float healthPercent, uint32 threshold)
    {
        return healthPercent <= static_cast<float>(threshold);
    }

    /**
     * @brief Check if health percentage is in a range
     * @param healthPercent Current health as percentage (0-100)
     * @param minValue Minimum value (inclusive)
     * @param maxValue Maximum value (exclusive)
     * @return true if health is in range [minValue, maxValue)
     */
    bool IsHealthInRange(float healthPercent, float minValue, float maxValue)
    {
        return healthPercent >= minValue && healthPercent < maxValue;
    }
};

TEST_F(HealthThresholdTest, CriticalHealthDetection)
{
    uint32 criticalThreshold = mockConfig.GetCriticalHealth();  // Default: 20

    // At or below critical
    EXPECT_TRUE(IsCriticalHealth(5.0f, criticalThreshold));
    EXPECT_TRUE(IsCriticalHealth(15.0f, criticalThreshold));
    EXPECT_TRUE(IsCriticalHealth(20.0f, criticalThreshold));

    // Above critical
    EXPECT_FALSE(IsCriticalHealth(21.0f, criticalThreshold));
    EXPECT_FALSE(IsCriticalHealth(50.0f, criticalThreshold));
    EXPECT_FALSE(IsCriticalHealth(100.0f, criticalThreshold));
}

TEST_F(HealthThresholdTest, LowHealthDetection)
{
    uint32 lowThreshold = mockConfig.GetLowHealth();  // Default: 45

    EXPECT_TRUE(IsCriticalHealth(30.0f, lowThreshold));
    EXPECT_TRUE(IsCriticalHealth(45.0f, lowThreshold));
    EXPECT_FALSE(IsCriticalHealth(46.0f, lowThreshold));
}

TEST_F(HealthThresholdTest, HealthRangeDetection)
{
    // Test "medium health" range (lowHealth to mediumHealth)
    uint32 lowHealth = mockConfig.GetLowHealth();        // 45
    uint32 mediumHealth = mockConfig.GetMediumHealth();  // 65

    // In range
    EXPECT_TRUE(IsHealthInRange(50.0f, lowHealth, mediumHealth));
    EXPECT_TRUE(IsHealthInRange(60.0f, lowHealth, mediumHealth));

    // Below range
    EXPECT_FALSE(IsHealthInRange(30.0f, lowHealth, mediumHealth));
    EXPECT_FALSE(IsHealthInRange(44.0f, lowHealth, mediumHealth));

    // Above range
    EXPECT_FALSE(IsHealthInRange(65.0f, lowHealth, mediumHealth));
    EXPECT_FALSE(IsHealthInRange(80.0f, lowHealth, mediumHealth));

    // At boundaries
    EXPECT_TRUE(IsHealthInRange(45.0f, lowHealth, mediumHealth));   // Min inclusive
    EXPECT_FALSE(IsHealthInRange(65.0f, lowHealth, mediumHealth));  // Max exclusive
}

TEST_F(HealthThresholdTest, AlmostFullHealthRange)
{
    uint32 mediumHealth = mockConfig.GetMediumHealth();        // 65
    uint32 almostFullHealth = mockConfig.GetAlmostFullHealth();  // 85

    // In "almost full" range
    EXPECT_TRUE(IsHealthInRange(70.0f, mediumHealth, almostFullHealth));
    EXPECT_TRUE(IsHealthInRange(80.0f, mediumHealth, almostFullHealth));

    // Below range
    EXPECT_FALSE(IsHealthInRange(60.0f, mediumHealth, almostFullHealth));

    // Above range (full health)
    EXPECT_FALSE(IsHealthInRange(90.0f, mediumHealth, almostFullHealth));
    EXPECT_FALSE(IsHealthInRange(100.0f, mediumHealth, almostFullHealth));
}

TEST_F(HealthThresholdTest, CustomThresholdsWithMock)
{
    using ::testing::Return;

    // Simulate a custom configuration with different thresholds
    EXPECT_CALL(mockConfig, GetCriticalHealth()).WillRepeatedly(Return(15u));
    EXPECT_CALL(mockConfig, GetLowHealth()).WillRepeatedly(Return(35u));
    EXPECT_CALL(mockConfig, GetMediumHealth()).WillRepeatedly(Return(55u));
    EXPECT_CALL(mockConfig, GetAlmostFullHealth()).WillRepeatedly(Return(75u));

    // Verify the custom thresholds
    EXPECT_EQ(15u, mockConfig.GetCriticalHealth());
    EXPECT_EQ(35u, mockConfig.GetLowHealth());
    EXPECT_EQ(55u, mockConfig.GetMediumHealth());
    EXPECT_EQ(75u, mockConfig.GetAlmostFullHealth());

    // Test with custom thresholds
    EXPECT_TRUE(IsCriticalHealth(15.0f, mockConfig.GetCriticalHealth()));
    EXPECT_FALSE(IsCriticalHealth(16.0f, mockConfig.GetCriticalHealth()));
}

/**
 * @brief Simulates the ValueInRangeTrigger logic
 */
class ValueInRangeTest : public ::testing::Test
{
protected:
    /**
     * @brief Check if value is in range (mirrors ValueInRangeTrigger::IsActive)
     *
     * From HealthTriggers.h:
     * bool IsActive() override {
     *     float value = GetValue();
     *     return value < maxValue && value >= minValue;
     * }
     */
    bool IsValueInRange(float value, float minValue, float maxValue)
    {
        return value < maxValue && value >= minValue;
    }
};

TEST_F(ValueInRangeTest, BasicRangeCheck)
{
    // Range [20, 45)
    EXPECT_FALSE(IsValueInRange(19.0f, 20.0f, 45.0f));  // Below min
    EXPECT_TRUE(IsValueInRange(20.0f, 20.0f, 45.0f));   // At min (inclusive)
    EXPECT_TRUE(IsValueInRange(30.0f, 20.0f, 45.0f));   // In range
    EXPECT_TRUE(IsValueInRange(44.9f, 20.0f, 45.0f));   // Just below max
    EXPECT_FALSE(IsValueInRange(45.0f, 20.0f, 45.0f));  // At max (exclusive)
    EXPECT_FALSE(IsValueInRange(50.0f, 20.0f, 45.0f));  // Above max
}

TEST_F(ValueInRangeTest, EdgeCases)
{
    // Empty range
    EXPECT_FALSE(IsValueInRange(10.0f, 10.0f, 10.0f));

    // Inverted range (min > max should always be false)
    EXPECT_FALSE(IsValueInRange(25.0f, 50.0f, 20.0f));

    // Zero-based range
    EXPECT_TRUE(IsValueInRange(0.0f, 0.0f, 100.0f));
    EXPECT_TRUE(IsValueInRange(50.0f, 0.0f, 100.0f));
    EXPECT_FALSE(IsValueInRange(100.0f, 0.0f, 100.0f));
}
