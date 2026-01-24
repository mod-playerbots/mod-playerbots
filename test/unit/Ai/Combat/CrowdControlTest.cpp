/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <set>
#include <algorithm>

/**
 * @brief Tests for crowd control management logic
 */

/**
 * @brief Tests for CC target selection
 */
class CcTargetSelectionTest : public ::testing::Test
{
protected:
    struct CcTarget
    {
        uint32 guid;
        std::string creatureType;  // "humanoid", "beast", "demon", etc.
        bool isElite;
        bool isBoss;
        float healthPercent;
        float distance;
        bool alreadyCCd;
        bool immune;
    };

    struct CcAbility
    {
        uint32 spellId;
        std::string name;
        std::set<std::string> validCreatureTypes;
        float maxRange;
        uint32 durationMs;
        bool breakOnDamage;
    };

    CcAbility ability_;
    std::vector<CcTarget> targets_;

    void SetUp() override
    {
        // Polymorph
        ability_ = {
            1,
            "Polymorph",
            {"humanoid", "beast", "critter"},
            30.0f,
            50000,
            true
        };

        targets_ = {
            {1, "humanoid", false, false, 100.0f, 20.0f, false, false},
            {2, "humanoid", true, false, 100.0f, 25.0f, false, false},
            {3, "demon", false, false, 100.0f, 15.0f, false, false},
            {4, "humanoid", false, true, 100.0f, 10.0f, false, false},  // Boss
            {5, "humanoid", false, false, 100.0f, 35.0f, false, false}, // Out of range
        };
    }

    bool CanCcTarget(CcTarget const& target, CcAbility const& ability)
    {
        if (target.immune)
            return false;
        if (target.isBoss)
            return false;
        if (target.alreadyCCd)
            return false;
        if (target.distance > ability.maxRange)
            return false;

        if (ability.validCreatureTypes.find(target.creatureType) == ability.validCreatureTypes.end())
            return false;

        return true;
    }

    std::vector<uint32> GetValidCcTargets()
    {
        std::vector<uint32> result;
        for (auto const& target : targets_)
        {
            if (CanCcTarget(target, ability_))
                result.push_back(target.guid);
        }
        return result;
    }

    uint32 SelectBestCcTarget()
    {
        CcTarget const* best = nullptr;
        float bestScore = -1.0f;

        for (auto const& target : targets_)
        {
            if (!CanCcTarget(target, ability_))
                continue;

            float score = 100.0f;

            // Prefer elites (more dangerous)
            if (target.isElite)
                score += 50.0f;

            // Prefer full health (will be active longer)
            score += target.healthPercent * 0.5f;

            // Prefer closer targets (faster CC application)
            score -= target.distance;

            if (score > bestScore)
            {
                best = &target;
                bestScore = score;
            }
        }

        return best ? best->guid : 0;
    }
};

TEST_F(CcTargetSelectionTest, ExcludesInvalidCreatureTypes)
{
    auto valid = GetValidCcTargets();

    bool hasDemon = std::find(valid.begin(), valid.end(), 3) != valid.end();
    EXPECT_FALSE(hasDemon);
}

TEST_F(CcTargetSelectionTest, ExcludesBosses)
{
    auto valid = GetValidCcTargets();

    bool hasBoss = std::find(valid.begin(), valid.end(), 4) != valid.end();
    EXPECT_FALSE(hasBoss);
}

TEST_F(CcTargetSelectionTest, ExcludesOutOfRange)
{
    auto valid = GetValidCcTargets();

    bool hasFar = std::find(valid.begin(), valid.end(), 5) != valid.end();
    EXPECT_FALSE(hasFar);
}

TEST_F(CcTargetSelectionTest, IncludesValidTargets)
{
    auto valid = GetValidCcTargets();

    bool hasNormal = std::find(valid.begin(), valid.end(), 1) != valid.end();
    bool hasElite = std::find(valid.begin(), valid.end(), 2) != valid.end();
    EXPECT_TRUE(hasNormal);
    EXPECT_TRUE(hasElite);
}

TEST_F(CcTargetSelectionTest, PrioritizesElites)
{
    uint32 best = SelectBestCcTarget();
    EXPECT_EQ(2u, best);  // Elite humanoid
}

TEST_F(CcTargetSelectionTest, ExcludesAlreadyCCd)
{
    targets_[1].alreadyCCd = true;  // Elite already CC'd
    uint32 best = SelectBestCcTarget();
    EXPECT_EQ(1u, best);  // Falls back to normal humanoid
}

/**
 * @brief Tests for CC break avoidance
 */
class CcBreakAvoidanceTest : public ::testing::Test
{
protected:
    struct CrowdControlledMob
    {
        uint32 guid;
        std::string ccType;
        uint32 remainingMs;
        bool breakOnDamage;
        bool breakOnMovement;
    };

    struct DamageAction
    {
        uint32 targetGuid;
        bool isAoe;
        float aoeRadius;
        float aoeX, aoeY;
    };

    struct Position
    {
        float x, y;
    };

    std::vector<CrowdControlledMob> ccMobs_;
    std::map<uint32, Position> mobPositions_;

    void SetUp() override
    {
        ccMobs_ = {
            {1, "polymorph", 30000, true, false},
            {2, "sap", 45000, true, true},
            {3, "fear", 8000, true, true},
        };

        mobPositions_ = {
            {1, {10.0f, 10.0f}},
            {2, {20.0f, 10.0f}},
            {3, {30.0f, 10.0f}},
            {100, {15.0f, 10.0f}},  // Non-CC'd target
        };
    }

    float Distance(Position const& a, Position const& b)
    {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    bool WouldBreakCc(DamageAction const& action)
    {
        for (auto const& cc : ccMobs_)
        {
            if (!cc.breakOnDamage)
                continue;

            // Direct damage
            if (action.targetGuid == cc.guid)
                return true;

            // AoE damage
            if (action.isAoe)
            {
                auto posIt = mobPositions_.find(cc.guid);
                if (posIt != mobPositions_.end())
                {
                    Position aoeCenter{action.aoeX, action.aoeY};
                    if (Distance(aoeCenter, posIt->second) <= action.aoeRadius)
                        return true;
                }
            }
        }

        return false;
    }

    bool IsSafeToAoe(float aoeX, float aoeY, float aoeRadius)
    {
        for (auto const& cc : ccMobs_)
        {
            if (!cc.breakOnDamage)
                continue;

            auto posIt = mobPositions_.find(cc.guid);
            if (posIt != mobPositions_.end())
            {
                Position aoeCenter{aoeX, aoeY};
                if (Distance(aoeCenter, posIt->second) <= aoeRadius)
                    return false;
            }
        }
        return true;
    }
};

TEST_F(CcBreakAvoidanceTest, DirectDamageBreaksCc)
{
    DamageAction direct{1, false, 0.0f, 0.0f, 0.0f};
    EXPECT_TRUE(WouldBreakCc(direct));
}

TEST_F(CcBreakAvoidanceTest, DirectDamageOnNonCcIsSafe)
{
    DamageAction direct{100, false, 0.0f, 0.0f, 0.0f};
    EXPECT_FALSE(WouldBreakCc(direct));
}

TEST_F(CcBreakAvoidanceTest, AoeDamageCanBreakCc)
{
    // AoE centered at 15, 10 with radius 10 would hit mob at 10, 10
    DamageAction aoe{0, true, 10.0f, 15.0f, 10.0f};
    EXPECT_TRUE(WouldBreakCc(aoe));
}

TEST_F(CcBreakAvoidanceTest, AoeAwayFromCcIsSafe)
{
    // AoE centered at 100, 100 - far from all CC'd mobs
    DamageAction aoe{0, true, 10.0f, 100.0f, 100.0f};
    EXPECT_FALSE(WouldBreakCc(aoe));
}

TEST_F(CcBreakAvoidanceTest, SafeAoeCheck)
{
    EXPECT_FALSE(IsSafeToAoe(15.0f, 10.0f, 10.0f));  // Would hit mob 1
    EXPECT_TRUE(IsSafeToAoe(100.0f, 100.0f, 10.0f)); // Safe location
}

/**
 * @brief Tests for CC duration tracking
 */
class CcDurationTrackingTest : public ::testing::Test
{
protected:
    struct CcInstance
    {
        uint32 mobGuid;
        uint32 spellId;
        uint32 appliedAt;
        uint32 baseDuration;
        float diminishingFactor;
    };

    std::vector<CcInstance> activeCC_;
    std::map<uint32, int> diminishingReturns_;  // mobGuid -> DR category count

    void ApplyCc(uint32 mobGuid, uint32 spellId, uint32 duration, uint32 currentTime, int drCategory)
    {
        // Calculate diminishing returns
        float drFactor = 1.0f;
        auto drIt = diminishingReturns_.find(mobGuid);
        if (drIt != diminishingReturns_.end())
        {
            int count = drIt->second;
            if (count == 1)
                drFactor = 0.5f;
            else if (count == 2)
                drFactor = 0.25f;
            else if (count >= 3)
                drFactor = 0.0f;  // Immune
        }

        if (drFactor > 0.0f)
        {
            activeCC_.push_back({mobGuid, spellId, currentTime, duration, drFactor});
        }

        // Increment DR
        diminishingReturns_[mobGuid]++;
    }

    uint32 GetRemainingDuration(uint32 mobGuid, uint32 currentTime)
    {
        for (auto const& cc : activeCC_)
        {
            if (cc.mobGuid == mobGuid)
            {
                uint32 effectiveDuration = static_cast<uint32>(cc.baseDuration * cc.diminishingFactor);
                uint32 endTime = cc.appliedAt + effectiveDuration;
                if (currentTime < endTime)
                    return endTime - currentTime;
            }
        }
        return 0;
    }

    bool IsImmune(uint32 mobGuid)
    {
        auto drIt = diminishingReturns_.find(mobGuid);
        if (drIt == diminishingReturns_.end())
            return false;
        return drIt->second >= 3;
    }

    void ResetDiminishingReturns()
    {
        diminishingReturns_.clear();
    }
};

TEST_F(CcDurationTrackingTest, FirstCcFullDuration)
{
    ApplyCc(1, 100, 10000, 0, 1);
    EXPECT_EQ(10000u, GetRemainingDuration(1, 0));
    EXPECT_EQ(5000u, GetRemainingDuration(1, 5000));
}

TEST_F(CcDurationTrackingTest, SecondCcHalfDuration)
{
    ApplyCc(1, 100, 10000, 0, 1);
    activeCC_.clear();  // Remove first CC
    ApplyCc(1, 100, 10000, 0, 1);
    EXPECT_EQ(5000u, GetRemainingDuration(1, 0));  // Half duration
}

TEST_F(CcDurationTrackingTest, ThirdCcQuarterDuration)
{
    ApplyCc(1, 100, 10000, 0, 1);
    activeCC_.clear();
    ApplyCc(1, 100, 10000, 0, 1);
    activeCC_.clear();
    ApplyCc(1, 100, 10000, 0, 1);
    EXPECT_EQ(2500u, GetRemainingDuration(1, 0));  // Quarter duration
}

TEST_F(CcDurationTrackingTest, FourthCcImmune)
{
    for (int i = 0; i < 3; i++)
    {
        ApplyCc(1, 100, 10000, 0, 1);
        activeCC_.clear();
    }

    EXPECT_TRUE(IsImmune(1));

    ApplyCc(1, 100, 10000, 0, 1);
    EXPECT_EQ(0u, GetRemainingDuration(1, 0));  // Immune
}

TEST_F(CcDurationTrackingTest, ResetDiminishing)
{
    for (int i = 0; i < 3; i++)
    {
        ApplyCc(1, 100, 10000, 0, 1);
        activeCC_.clear();
    }

    EXPECT_TRUE(IsImmune(1));

    ResetDiminishingReturns();

    EXPECT_FALSE(IsImmune(1));
}

