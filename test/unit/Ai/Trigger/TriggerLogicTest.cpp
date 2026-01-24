/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

/**
 * @brief Tests for trigger logic patterns
 *
 * These tests validate the core logic patterns used by triggers
 * without requiring actual game objects.
 */

/**
 * @brief Simulates basic trigger activation logic
 */
class TriggerLogicTest : public ::testing::Test
{
protected:
    /**
     * @brief Check if a threshold trigger should activate
     * @param currentValue Current value to check
     * @param threshold Threshold to check against
     * @return true if current value is at or below threshold
     */
    bool IsThresholdTriggered(float currentValue, float threshold)
    {
        return currentValue <= threshold;
    }

    /**
     * @brief Check if value is in range (ValueInRangeTrigger pattern)
     * @param value Current value
     * @param minValue Minimum value (inclusive)
     * @param maxValue Maximum value (exclusive)
     * @return true if value is in range [minValue, maxValue)
     */
    bool IsInRange(float value, float minValue, float maxValue)
    {
        return value >= minValue && value < maxValue;
    }

    /**
     * @brief Check if cooldown has elapsed
     * @param lastActivation Time of last activation
     * @param currentTime Current time
     * @param cooldown Cooldown duration
     * @return true if cooldown has elapsed
     */
    bool IsCooldownElapsed(uint32 lastActivation, uint32 currentTime, uint32 cooldown)
    {
        if (lastActivation == 0)
            return true;  // Never activated before
        return (currentTime - lastActivation) >= cooldown;
    }
};

TEST_F(TriggerLogicTest, ThresholdTriggerActivation)
{
    float const criticalThreshold = 20.0f;

    // Should trigger when at or below threshold
    EXPECT_TRUE(IsThresholdTriggered(5.0f, criticalThreshold));
    EXPECT_TRUE(IsThresholdTriggered(15.0f, criticalThreshold));
    EXPECT_TRUE(IsThresholdTriggered(20.0f, criticalThreshold));

    // Should not trigger when above threshold
    EXPECT_FALSE(IsThresholdTriggered(21.0f, criticalThreshold));
    EXPECT_FALSE(IsThresholdTriggered(50.0f, criticalThreshold));
    EXPECT_FALSE(IsThresholdTriggered(100.0f, criticalThreshold));
}

TEST_F(TriggerLogicTest, RangeTriggerActivation)
{
    // Range [20, 45)
    float const minValue = 20.0f;
    float const maxValue = 45.0f;

    // In range
    EXPECT_TRUE(IsInRange(20.0f, minValue, maxValue));  // At min (inclusive)
    EXPECT_TRUE(IsInRange(30.0f, minValue, maxValue));  // Middle
    EXPECT_TRUE(IsInRange(44.9f, minValue, maxValue));  // Just below max

    // Out of range
    EXPECT_FALSE(IsInRange(19.9f, minValue, maxValue)); // Below min
    EXPECT_FALSE(IsInRange(45.0f, minValue, maxValue)); // At max (exclusive)
    EXPECT_FALSE(IsInRange(50.0f, minValue, maxValue)); // Above max
}

TEST_F(TriggerLogicTest, CooldownCheck)
{
    uint32 const cooldown = 5000;  // 5 seconds

    // Never activated - should be ready
    EXPECT_TRUE(IsCooldownElapsed(0, 10000, cooldown));

    // Just activated - should not be ready
    EXPECT_FALSE(IsCooldownElapsed(10000, 10000, cooldown));

    // Not enough time passed
    EXPECT_FALSE(IsCooldownElapsed(10000, 13000, cooldown));

    // Exactly at cooldown - should be ready
    EXPECT_TRUE(IsCooldownElapsed(10000, 15000, cooldown));

    // Past cooldown - should be ready
    EXPECT_TRUE(IsCooldownElapsed(10000, 20000, cooldown));
}

/**
 * @brief Tests for multi-condition trigger logic
 */
class MultiConditionTriggerTest : public ::testing::Test
{
protected:
    struct TriggerConditions
    {
        bool hasTarget;
        bool inCombat;
        bool hasEnoughMana;
        float healthPercent;
        float distanceToTarget;
    };

    /**
     * @brief Example: Heal spell trigger logic
     */
    bool ShouldCastHeal(TriggerConditions const& cond, float healThreshold, float maxRange)
    {
        return cond.hasTarget &&
               cond.hasEnoughMana &&
               cond.healthPercent <= healThreshold &&
               cond.distanceToTarget <= maxRange;
    }

    /**
     * @brief Example: Attack spell trigger logic
     */
    bool ShouldCastAttack(TriggerConditions const& cond, float maxRange)
    {
        return cond.hasTarget &&
               cond.inCombat &&
               cond.hasEnoughMana &&
               cond.distanceToTarget <= maxRange;
    }
};

TEST_F(MultiConditionTriggerTest, HealTriggerConditions)
{
    float const healThreshold = 45.0f;
    float const maxRange = 40.0f;

    // All conditions met
    TriggerConditions goodConditions{true, true, true, 30.0f, 20.0f};
    EXPECT_TRUE(ShouldCastHeal(goodConditions, healThreshold, maxRange));

    // No target
    TriggerConditions noTarget{false, true, true, 30.0f, 20.0f};
    EXPECT_FALSE(ShouldCastHeal(noTarget, healThreshold, maxRange));

    // Not enough mana
    TriggerConditions noMana{true, true, false, 30.0f, 20.0f};
    EXPECT_FALSE(ShouldCastHeal(noMana, healThreshold, maxRange));

    // Health above threshold
    TriggerConditions healthyTarget{true, true, true, 80.0f, 20.0f};
    EXPECT_FALSE(ShouldCastHeal(healthyTarget, healThreshold, maxRange));

    // Out of range
    TriggerConditions outOfRange{true, true, true, 30.0f, 50.0f};
    EXPECT_FALSE(ShouldCastHeal(outOfRange, healThreshold, maxRange));
}

TEST_F(MultiConditionTriggerTest, AttackTriggerConditions)
{
    float const maxRange = 30.0f;

    // All conditions met
    TriggerConditions combatReady{true, true, true, 100.0f, 20.0f};
    EXPECT_TRUE(ShouldCastAttack(combatReady, maxRange));

    // Not in combat
    TriggerConditions noCombat{true, false, true, 100.0f, 20.0f};
    EXPECT_FALSE(ShouldCastAttack(noCombat, maxRange));

    // Out of range
    TriggerConditions tooFar{true, true, true, 100.0f, 40.0f};
    EXPECT_FALSE(ShouldCastAttack(tooFar, maxRange));
}

/**
 * @brief Tests for priority-based trigger selection
 */
class TriggerPriorityTest : public ::testing::Test
{
protected:
    struct TriggerEntry
    {
        std::string name;
        float priority;
        bool isActive;
    };

    /**
     * @brief Find highest priority active trigger
     */
    std::string FindHighestPriorityTrigger(std::vector<TriggerEntry> const& triggers)
    {
        TriggerEntry const* best = nullptr;

        for (auto const& trigger : triggers)
        {
            if (!trigger.isActive)
                continue;

            if (!best || trigger.priority > best->priority)
                best = &trigger;
        }

        return best ? best->name : "";
    }
};

TEST_F(TriggerPriorityTest, SelectsHighestPriority)
{
    std::vector<TriggerEntry> triggers = {
        {"low_priority", 10.0f, true},
        {"high_priority", 100.0f, true},
        {"medium_priority", 50.0f, true},
    };

    EXPECT_EQ("high_priority", FindHighestPriorityTrigger(triggers));
}

TEST_F(TriggerPriorityTest, SkipsInactiveTriggers)
{
    std::vector<TriggerEntry> triggers = {
        {"low_priority", 10.0f, true},
        {"high_priority", 100.0f, false},  // Inactive
        {"medium_priority", 50.0f, true},
    };

    EXPECT_EQ("medium_priority", FindHighestPriorityTrigger(triggers));
}

TEST_F(TriggerPriorityTest, ReturnsEmptyWhenNoActive)
{
    std::vector<TriggerEntry> triggers = {
        {"trigger1", 10.0f, false},
        {"trigger2", 100.0f, false},
    };

    EXPECT_EQ("", FindHighestPriorityTrigger(triggers));
}

