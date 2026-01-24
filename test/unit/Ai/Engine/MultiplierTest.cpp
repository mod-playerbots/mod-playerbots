/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <functional>
#include <cmath>

/**
 * @brief Tests for action priority multiplier calculations
 */

/**
 * @brief Tests for basic multiplier calculations
 */
class MultiplierCalculationTest : public ::testing::Test
{
protected:
    struct MultiplierContext
    {
        float healthPercent;
        float manaPercent;
        float targetHealthPercent;
        bool inCombat;
        bool hasAggro;
        int groupSize;
        float threatPercent;
    };

    MultiplierContext context_;

    void SetUp() override
    {
        context_ = {100.0f, 100.0f, 100.0f, true, false, 5, 50.0f};
    }

    /**
     * @brief Calculate heal urgency multiplier
     * Lower health = higher multiplier
     */
    float GetHealUrgencyMultiplier(float healthPercent)
    {
        if (healthPercent >= 80.0f)
            return 1.0f;
        if (healthPercent >= 50.0f)
            return 1.5f;
        if (healthPercent >= 30.0f)
            return 2.0f;
        if (healthPercent >= 15.0f)
            return 3.0f;
        return 5.0f;  // Critical
    }

    /**
     * @brief Calculate threat management multiplier
     * Higher threat = higher priority for threat reduction
     */
    float GetThreatMultiplier(float threatPercent, bool isTank)
    {
        if (isTank)
            return 1.0f;  // Tanks don't reduce threat

        if (threatPercent >= 100.0f)
            return 5.0f;  // Emergency!
        if (threatPercent >= 90.0f)
            return 3.0f;
        if (threatPercent >= 80.0f)
            return 2.0f;
        if (threatPercent >= 70.0f)
            return 1.5f;
        return 1.0f;
    }

    /**
     * @brief Calculate mana conservation multiplier
     * Lower mana = prefer efficient spells
     */
    float GetManaConservationMultiplier(float manaPercent)
    {
        if (manaPercent >= 80.0f)
            return 1.0f;
        if (manaPercent >= 50.0f)
            return 1.2f;
        if (manaPercent >= 30.0f)
            return 1.5f;
        if (manaPercent >= 15.0f)
            return 2.0f;
        return 3.0f;  // Very low mana
    }

    /**
     * @brief Calculate group size scaling multiplier
     */
    float GetGroupScalingMultiplier(int groupSize, bool isAoeAction)
    {
        if (!isAoeAction)
            return 1.0f;

        if (groupSize >= 20)  // Raid
            return 2.0f;
        if (groupSize >= 10)
            return 1.5f;
        if (groupSize >= 5)
            return 1.2f;
        return 1.0f;
    }
};

TEST_F(MultiplierCalculationTest, HealUrgencyAtFullHealth)
{
    EXPECT_FLOAT_EQ(1.0f, GetHealUrgencyMultiplier(100.0f));
    EXPECT_FLOAT_EQ(1.0f, GetHealUrgencyMultiplier(80.0f));
}

TEST_F(MultiplierCalculationTest, HealUrgencyScalesWithDamage)
{
    EXPECT_FLOAT_EQ(1.5f, GetHealUrgencyMultiplier(60.0f));
    EXPECT_FLOAT_EQ(2.0f, GetHealUrgencyMultiplier(40.0f));
    EXPECT_FLOAT_EQ(3.0f, GetHealUrgencyMultiplier(20.0f));
}

TEST_F(MultiplierCalculationTest, HealUrgencyCritical)
{
    EXPECT_FLOAT_EQ(5.0f, GetHealUrgencyMultiplier(10.0f));
    EXPECT_FLOAT_EQ(5.0f, GetHealUrgencyMultiplier(5.0f));
}

TEST_F(MultiplierCalculationTest, ThreatMultiplierForDps)
{
    EXPECT_FLOAT_EQ(1.0f, GetThreatMultiplier(50.0f, false));
    EXPECT_FLOAT_EQ(1.5f, GetThreatMultiplier(75.0f, false));
    EXPECT_FLOAT_EQ(2.0f, GetThreatMultiplier(85.0f, false));
    EXPECT_FLOAT_EQ(3.0f, GetThreatMultiplier(95.0f, false));
    EXPECT_FLOAT_EQ(5.0f, GetThreatMultiplier(110.0f, false));
}

TEST_F(MultiplierCalculationTest, TankIgnoresThreatMultiplier)
{
    EXPECT_FLOAT_EQ(1.0f, GetThreatMultiplier(100.0f, true));
    EXPECT_FLOAT_EQ(1.0f, GetThreatMultiplier(150.0f, true));
}

TEST_F(MultiplierCalculationTest, ManaConservation)
{
    EXPECT_FLOAT_EQ(1.0f, GetManaConservationMultiplier(100.0f));
    EXPECT_FLOAT_EQ(1.2f, GetManaConservationMultiplier(60.0f));
    EXPECT_FLOAT_EQ(1.5f, GetManaConservationMultiplier(40.0f));
    EXPECT_FLOAT_EQ(2.0f, GetManaConservationMultiplier(20.0f));
    EXPECT_FLOAT_EQ(3.0f, GetManaConservationMultiplier(10.0f));
}

TEST_F(MultiplierCalculationTest, GroupScalingForAoe)
{
    EXPECT_FLOAT_EQ(1.0f, GetGroupScalingMultiplier(3, true));
    EXPECT_FLOAT_EQ(1.2f, GetGroupScalingMultiplier(5, true));
    EXPECT_FLOAT_EQ(1.5f, GetGroupScalingMultiplier(10, true));
    EXPECT_FLOAT_EQ(2.0f, GetGroupScalingMultiplier(25, true));
}

TEST_F(MultiplierCalculationTest, GroupScalingIgnoredForSingleTarget)
{
    EXPECT_FLOAT_EQ(1.0f, GetGroupScalingMultiplier(25, false));
}

/**
 * @brief Tests for combined multiplier calculations
 */
class CombinedMultiplierTest : public ::testing::Test
{
protected:
    struct ActionPriority
    {
        std::string name;
        float basePriority;
        std::vector<float> multipliers;
    };

    /**
     * @brief Calculate final priority with all multipliers
     */
    float CalculateFinalPriority(ActionPriority const& action)
    {
        float finalPriority = action.basePriority;
        for (float mult : action.multipliers)
        {
            finalPriority *= mult;
        }
        return finalPriority;
    }

    /**
     * @brief Compare actions and select highest priority
     */
    std::string SelectBestAction(std::vector<ActionPriority> const& actions)
    {
        ActionPriority const* best = nullptr;
        float bestPriority = -1.0f;

        for (auto const& action : actions)
        {
            float priority = CalculateFinalPriority(action);
            if (priority > bestPriority)
            {
                best = &action;
                bestPriority = priority;
            }
        }

        return best ? best->name : "";
    }
};

TEST_F(CombinedMultiplierTest, SingleMultiplier)
{
    ActionPriority action{"test", 100.0f, {2.0f}};
    EXPECT_FLOAT_EQ(200.0f, CalculateFinalPriority(action));
}

TEST_F(CombinedMultiplierTest, MultipleMultipliers)
{
    ActionPriority action{"test", 100.0f, {2.0f, 1.5f, 1.2f}};
    EXPECT_FLOAT_EQ(360.0f, CalculateFinalPriority(action));
}

TEST_F(CombinedMultiplierTest, NoMultipliers)
{
    ActionPriority action{"test", 100.0f, {}};
    EXPECT_FLOAT_EQ(100.0f, CalculateFinalPriority(action));
}

TEST_F(CombinedMultiplierTest, MultiplierAffectsSelection)
{
    std::vector<ActionPriority> actions = {
        {"heal", 50.0f, {3.0f}},   // 150
        {"attack", 100.0f, {1.0f}}, // 100
    };

    EXPECT_EQ("heal", SelectBestAction(actions));
}

TEST_F(CombinedMultiplierTest, HighBaseCanBeatMultiplied)
{
    std::vector<ActionPriority> actions = {
        {"heal", 50.0f, {1.5f}},    // 75
        {"attack", 100.0f, {1.0f}}, // 100
    };

    EXPECT_EQ("attack", SelectBestAction(actions));
}

/**
 * @brief Tests for situational multipliers
 */
class SituationalMultiplierTest : public ::testing::Test
{
protected:
    struct BossPhase
    {
        int phase;
        bool isBurning;      // High damage phase
        bool needsInterrupt;
        bool needsSpread;
        bool needsStack;
    };

    BossPhase currentPhase_;

    void SetUp() override
    {
        currentPhase_ = {1, false, false, false, false};
    }

    float GetBossPhaseMultiplier(std::string const& actionType)
    {
        if (currentPhase_.isBurning)
        {
            if (actionType == "heal")
                return 2.0f;
            if (actionType == "defensive_cooldown")
                return 3.0f;
        }

        if (currentPhase_.needsInterrupt && actionType == "interrupt")
            return 5.0f;

        if (currentPhase_.needsSpread && actionType == "spread")
            return 4.0f;

        if (currentPhase_.needsStack && actionType == "stack")
            return 4.0f;

        return 1.0f;
    }

    float GetExecutePhaseMultiplier(float targetHealthPercent, std::string const& actionType)
    {
        if (targetHealthPercent <= 20.0f && actionType == "execute")
            return 3.0f;
        if (targetHealthPercent <= 35.0f && actionType == "execute")
            return 2.0f;
        return 1.0f;
    }
};

TEST_F(SituationalMultiplierTest, NormalPhaseNoBonus)
{
    EXPECT_FLOAT_EQ(1.0f, GetBossPhaseMultiplier("heal"));
    EXPECT_FLOAT_EQ(1.0f, GetBossPhaseMultiplier("attack"));
}

TEST_F(SituationalMultiplierTest, BurningPhaseBoostsHealing)
{
    currentPhase_.isBurning = true;
    EXPECT_FLOAT_EQ(2.0f, GetBossPhaseMultiplier("heal"));
    EXPECT_FLOAT_EQ(3.0f, GetBossPhaseMultiplier("defensive_cooldown"));
}

TEST_F(SituationalMultiplierTest, InterruptPhaseBoostsInterrupt)
{
    currentPhase_.needsInterrupt = true;
    EXPECT_FLOAT_EQ(5.0f, GetBossPhaseMultiplier("interrupt"));
}

TEST_F(SituationalMultiplierTest, SpreadStackMechanics)
{
    currentPhase_.needsSpread = true;
    EXPECT_FLOAT_EQ(4.0f, GetBossPhaseMultiplier("spread"));

    currentPhase_.needsSpread = false;
    currentPhase_.needsStack = true;
    EXPECT_FLOAT_EQ(4.0f, GetBossPhaseMultiplier("stack"));
}

TEST_F(SituationalMultiplierTest, ExecutePhase)
{
    EXPECT_FLOAT_EQ(1.0f, GetExecutePhaseMultiplier(50.0f, "execute"));
    EXPECT_FLOAT_EQ(2.0f, GetExecutePhaseMultiplier(30.0f, "execute"));
    EXPECT_FLOAT_EQ(3.0f, GetExecutePhaseMultiplier(15.0f, "execute"));
}

