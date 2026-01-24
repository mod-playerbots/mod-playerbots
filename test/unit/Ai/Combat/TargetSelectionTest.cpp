/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

/**
 * @brief Tests for target selection and prioritization logic
 */

/**
 * @brief Tests for enemy target prioritization
 */
class EnemyPrioritizationTest : public ::testing::Test
{
protected:
    struct EnemyTarget
    {
        uint32 guid;
        std::string name;
        float healthPercent;
        float distance;
        bool isCasting;
        bool isElite;
        bool isBoss;
        bool hasAggro;
        float threatToTank;
        bool isCrowdControlled;
        int debuffCount;
    };

    struct TargetContext
    {
        bool isTank;
        bool isInterrupter;
        uint32 currentTargetGuid;
    };

    TargetContext context_;

    void SetUp() override
    {
        context_ = {false, false, 0};
    }

    /**
     * @brief Calculate target priority score
     */
    float CalculatePriority(EnemyTarget const& target, TargetContext const& ctx)
    {
        if (target.isCrowdControlled)
            return -1000.0f;  // Never attack CC'd targets

        float score = 100.0f;

        // Current target bonus (reduce target switching)
        if (target.guid == ctx.currentTargetGuid)
            score += 20.0f;

        // Health-based priority (lower health = higher priority for focus fire)
        score += (100.0f - target.healthPercent) * 0.5f;

        // Distance penalty
        score -= target.distance * 0.5f;

        // Boss/Elite priority
        if (target.isBoss)
            score += 50.0f;
        else if (target.isElite)
            score += 25.0f;

        // Interrupt priority
        if (target.isCasting && ctx.isInterrupter)
            score += 100.0f;

        // Tank threat considerations
        if (ctx.isTank)
        {
            // Prioritize loose mobs (not on tank)
            if (!target.hasAggro)
                score += 75.0f;
        }
        else
        {
            // DPS should attack tank's target
            if (target.hasAggro)
                score += 30.0f;
        }

        // Debuff stacking bonus
        score += target.debuffCount * 5.0f;

        return score;
    }

    /**
     * @brief Select best target from list
     */
    uint32 SelectBestTarget(std::vector<EnemyTarget> const& enemies)
    {
        EnemyTarget const* best = nullptr;
        float bestScore = -9999.0f;

        for (auto const& enemy : enemies)
        {
            float score = CalculatePriority(enemy, context_);
            if (score > bestScore)
            {
                best = &enemy;
                bestScore = score;
            }
        }

        return best ? best->guid : 0;
    }
};

TEST_F(EnemyPrioritizationTest, AvoidsCrowdControlled)
{
    std::vector<EnemyTarget> enemies = {
        {1, "mob1", 50.0f, 10.0f, false, false, false, true, 100.0f, true, 0},  // CC'd
        {2, "mob2", 100.0f, 20.0f, false, false, false, true, 100.0f, false, 0}, // Not CC'd
    };

    EXPECT_EQ(2u, SelectBestTarget(enemies));
}

TEST_F(EnemyPrioritizationTest, PrioritizesLowHealth)
{
    std::vector<EnemyTarget> enemies = {
        {1, "mob1", 80.0f, 10.0f, false, false, false, true, 100.0f, false, 0},
        {2, "mob2", 20.0f, 10.0f, false, false, false, true, 100.0f, false, 0},
    };

    EXPECT_EQ(2u, SelectBestTarget(enemies));
}

TEST_F(EnemyPrioritizationTest, PrioritizesBoss)
{
    std::vector<EnemyTarget> enemies = {
        {1, "mob", 50.0f, 10.0f, false, false, false, true, 100.0f, false, 0},
        {2, "boss", 100.0f, 10.0f, false, false, true, true, 100.0f, false, 0},
    };

    EXPECT_EQ(2u, SelectBestTarget(enemies));
}

TEST_F(EnemyPrioritizationTest, InterrupterPrioritizesCaster)
{
    context_.isInterrupter = true;

    std::vector<EnemyTarget> enemies = {
        {1, "melee", 30.0f, 10.0f, false, false, false, true, 100.0f, false, 0},
        {2, "caster", 80.0f, 10.0f, true, false, false, true, 100.0f, false, 0},
    };

    EXPECT_EQ(2u, SelectBestTarget(enemies));
}

TEST_F(EnemyPrioritizationTest, TankPrioritizesLooseMobs)
{
    context_.isTank = true;

    std::vector<EnemyTarget> enemies = {
        {1, "controlled", 50.0f, 10.0f, false, false, false, true, 100.0f, false, 0},
        {2, "loose", 50.0f, 10.0f, false, false, false, false, 0.0f, false, 0},
    };

    EXPECT_EQ(2u, SelectBestTarget(enemies));
}

TEST_F(EnemyPrioritizationTest, DpsPrioritizesTankTarget)
{
    context_.isTank = false;

    std::vector<EnemyTarget> enemies = {
        {1, "tanked", 80.0f, 10.0f, false, false, false, true, 100.0f, false, 0},
        {2, "loose", 50.0f, 10.0f, false, false, false, false, 0.0f, false, 0},
    };

    EXPECT_EQ(1u, SelectBestTarget(enemies));
}

TEST_F(EnemyPrioritizationTest, CurrentTargetBonus)
{
    context_.currentTargetGuid = 1;

    std::vector<EnemyTarget> enemies = {
        {1, "current", 70.0f, 10.0f, false, false, false, true, 100.0f, false, 0},
        {2, "other", 65.0f, 10.0f, false, false, false, true, 100.0f, false, 0},
    };

    // Current target slightly higher health but should still be selected
    EXPECT_EQ(1u, SelectBestTarget(enemies));
}

/**
 * @brief Tests for healing target selection
 */
class HealTargetSelectionTest : public ::testing::Test
{
protected:
    struct HealTarget
    {
        uint32 guid;
        std::string name;
        float healthPercent;
        float distance;
        bool isTank;
        bool isHealer;
        bool isDps;
        bool hasCriticalDebuff;
        bool isInLineOfSight;
        int incomingHeals;
    };

    /**
     * @brief Calculate heal priority
     */
    float CalculateHealPriority(HealTarget const& target, float maxRange)
    {
        if (!target.isInLineOfSight)
            return -1000.0f;

        if (target.distance > maxRange)
            return -1000.0f;

        float score = 100.0f;

        // Health deficit is primary factor
        float healthDeficit = 100.0f - target.healthPercent;
        score += healthDeficit * 2.0f;

        // Role priority: Tank > Healer > DPS
        if (target.isTank)
            score += 50.0f;
        else if (target.isHealer)
            score += 30.0f;

        // Critical debuff bonus
        if (target.hasCriticalDebuff)
            score += 40.0f;

        // Incoming heals penalty (avoid overhealing)
        score -= target.incomingHeals * 10.0f;

        // Distance penalty (prefer closer targets)
        score -= target.distance * 0.5f;

        return score;
    }

    uint32 SelectHealTarget(std::vector<HealTarget> const& targets, float maxRange)
    {
        HealTarget const* best = nullptr;
        float bestScore = -9999.0f;

        for (auto const& target : targets)
        {
            float score = CalculateHealPriority(target, maxRange);
            if (score > bestScore)
            {
                best = &target;
                bestScore = score;
            }
        }

        return best ? best->guid : 0;
    }
};

TEST_F(HealTargetSelectionTest, PrioritizesLowHealth)
{
    std::vector<HealTarget> targets = {
        {1, "healthy", 90.0f, 10.0f, false, false, true, false, true, 0},
        {2, "injured", 30.0f, 10.0f, false, false, true, false, true, 0},
    };

    EXPECT_EQ(2u, SelectHealTarget(targets, 40.0f));
}

TEST_F(HealTargetSelectionTest, PrioritizesTank)
{
    std::vector<HealTarget> targets = {
        {1, "dps", 50.0f, 10.0f, false, false, true, false, true, 0},
        {2, "tank", 60.0f, 10.0f, true, false, false, false, true, 0},
    };

    EXPECT_EQ(2u, SelectHealTarget(targets, 40.0f));
}

TEST_F(HealTargetSelectionTest, SkipsOutOfRange)
{
    std::vector<HealTarget> targets = {
        {1, "far", 10.0f, 50.0f, false, false, true, false, true, 0},
        {2, "close", 80.0f, 10.0f, false, false, true, false, true, 0},
    };

    EXPECT_EQ(2u, SelectHealTarget(targets, 40.0f));
}

TEST_F(HealTargetSelectionTest, SkipsOutOfSight)
{
    std::vector<HealTarget> targets = {
        {1, "hidden", 10.0f, 10.0f, false, false, true, false, false, 0},
        {2, "visible", 80.0f, 10.0f, false, false, true, false, true, 0},
    };

    EXPECT_EQ(2u, SelectHealTarget(targets, 40.0f));
}

TEST_F(HealTargetSelectionTest, AccountsForIncomingHeals)
{
    std::vector<HealTarget> targets = {
        {1, "being_healed", 40.0f, 10.0f, false, false, true, false, true, 3},
        {2, "not_healed", 50.0f, 10.0f, false, false, true, false, true, 0},
    };

    EXPECT_EQ(2u, SelectHealTarget(targets, 40.0f));
}

TEST_F(HealTargetSelectionTest, CriticalDebuffBonus)
{
    std::vector<HealTarget> targets = {
        {1, "normal", 50.0f, 10.0f, false, false, true, false, true, 0},
        {2, "debuffed", 60.0f, 10.0f, false, false, true, true, true, 0},
    };

    EXPECT_EQ(2u, SelectHealTarget(targets, 40.0f));
}

/**
 * @brief Tests for AoE target selection
 */
class AoeTargetSelectionTest : public ::testing::Test
{
protected:
    struct Position
    {
        float x, y;
    };

    struct AoeTarget
    {
        uint32 guid;
        Position pos;
        bool isCrowdControlled;
    };

    float Distance(Position const& a, Position const& b)
    {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    /**
     * @brief Count targets that would be hit by AoE at position
     */
    int CountTargetsInAoe(std::vector<AoeTarget> const& targets, Position const& center, float radius)
    {
        int count = 0;
        for (auto const& target : targets)
        {
            if (target.isCrowdControlled)
                continue;
            if (Distance(center, target.pos) <= radius)
                count++;
        }
        return count;
    }

    /**
     * @brief Find best AoE center position
     */
    Position FindBestAoePosition(std::vector<AoeTarget> const& targets, float radius)
    {
        Position bestPos{0.0f, 0.0f};
        int bestCount = 0;

        // Simple approach: check each target as potential center
        for (auto const& target : targets)
        {
            if (target.isCrowdControlled)
                continue;

            int count = CountTargetsInAoe(targets, target.pos, radius);
            if (count > bestCount)
            {
                bestCount = count;
                bestPos = target.pos;
            }
        }

        return bestPos;
    }

    /**
     * @brief Check if AoE is worth using
     */
    bool ShouldUseAoe(std::vector<AoeTarget> const& targets, float radius, int minTargets)
    {
        // Find max targets possible
        int maxTargets = 0;
        for (auto const& target : targets)
        {
            if (target.isCrowdControlled)
                continue;

            int count = CountTargetsInAoe(targets, target.pos, radius);
            maxTargets = std::max(maxTargets, count);
        }

        return maxTargets >= minTargets;
    }
};

TEST_F(AoeTargetSelectionTest, CountsTargetsInRadius)
{
    std::vector<AoeTarget> targets = {
        {1, {0.0f, 0.0f}, false},
        {2, {5.0f, 0.0f}, false},
        {3, {20.0f, 0.0f}, false},
    };

    EXPECT_EQ(2, CountTargetsInAoe(targets, {0.0f, 0.0f}, 10.0f));
}

TEST_F(AoeTargetSelectionTest, ExcludesCrowdControlled)
{
    std::vector<AoeTarget> targets = {
        {1, {0.0f, 0.0f}, false},
        {2, {5.0f, 0.0f}, true},  // CC'd
        {3, {3.0f, 0.0f}, false},
    };

    EXPECT_EQ(2, CountTargetsInAoe(targets, {0.0f, 0.0f}, 10.0f));
}

TEST_F(AoeTargetSelectionTest, FindsBestCenter)
{
    std::vector<AoeTarget> targets = {
        {1, {0.0f, 0.0f}, false},
        {2, {5.0f, 5.0f}, false},
        {3, {6.0f, 5.0f}, false},
        {4, {5.0f, 6.0f}, false},
        {5, {50.0f, 50.0f}, false},
    };

    Position best = FindBestAoePosition(targets, 5.0f);
    // Best position should be near the cluster of 3
    EXPECT_GT(best.x, 3.0f);
    EXPECT_LT(best.x, 8.0f);
}

TEST_F(AoeTargetSelectionTest, AoeThresholdMet)
{
    std::vector<AoeTarget> targets = {
        {1, {0.0f, 0.0f}, false},
        {2, {5.0f, 0.0f}, false},
        {3, {3.0f, 0.0f}, false},
    };

    EXPECT_TRUE(ShouldUseAoe(targets, 10.0f, 3));
    EXPECT_FALSE(ShouldUseAoe(targets, 10.0f, 4));
}

