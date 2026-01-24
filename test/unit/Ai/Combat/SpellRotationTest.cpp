/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

/**
 * @brief Tests for spell rotation and selection logic
 */

/**
 * @brief Tests for basic spell selection
 */
class SpellSelectionTest : public ::testing::Test
{
protected:
    struct SpellInfo
    {
        uint32 id;
        std::string name;
        uint32 manaCost;
        uint32 cooldown;      // ms
        uint32 castTime;      // ms
        float minRange;
        float maxRange;
        float damage;
        bool requiresBehind;
        bool isAoe;
        int comboPointCost;
        int comboPointsGenerated;
    };

    struct SpellState
    {
        uint32 cooldownRemaining;
        bool isUsable;
    };

    struct CastContext
    {
        float currentMana;
        float maxMana;
        float targetDistance;
        bool isBehindTarget;
        int comboPoints;
        bool targetIsMoving;
        int enemyCount;
    };

    std::map<uint32, SpellInfo> spells_;
    std::map<uint32, SpellState> spellStates_;
    CastContext context_;

    void SetUp() override
    {
        // Define test spells
        spells_[1] = {1, "Sinister Strike", 45, 0, 0, 0, 5, 100, false, false, 0, 1};
        spells_[2] = {2, "Backstab", 60, 0, 0, 0, 5, 200, true, false, 0, 1};
        spells_[3] = {3, "Eviscerate", 35, 0, 0, 0, 5, 500, false, false, 5, 0};
        spells_[4] = {4, "Fan of Knives", 50, 10000, 0, 0, 10, 150, false, true, 0, 0};
        spells_[5] = {5, "Ambush", 60, 0, 0, 0, 5, 300, true, false, 0, 2};

        // All spells ready by default
        for (auto const& [id, _] : spells_)
        {
            spellStates_[id] = {0, true};
        }

        context_ = {100.0f, 100.0f, 3.0f, false, 0, false, 1};
    }

    bool CanCastSpell(uint32 spellId)
    {
        auto spellIt = spells_.find(spellId);
        if (spellIt == spells_.end())
            return false;

        auto stateIt = spellStates_.find(spellId);
        if (stateIt == spellStates_.end())
            return false;

        SpellInfo const& spell = spellIt->second;
        SpellState const& state = stateIt->second;

        // Cooldown check
        if (state.cooldownRemaining > 0)
            return false;

        // Mana check
        if (context_.currentMana < spell.manaCost)
            return false;

        // Range check
        if (context_.targetDistance < spell.minRange || context_.targetDistance > spell.maxRange)
            return false;

        // Position check
        if (spell.requiresBehind && !context_.isBehindTarget)
            return false;

        // Combo point check
        if (spell.comboPointCost > context_.comboPoints)
            return false;

        // Moving target + cast time
        if (context_.targetIsMoving && spell.castTime > 0)
            return false;

        return state.isUsable;
    }

    std::vector<uint32> GetUsableSpells()
    {
        std::vector<uint32> result;
        for (auto const& [id, _] : spells_)
        {
            if (CanCastSpell(id))
                result.push_back(id);
        }
        return result;
    }
};

TEST_F(SpellSelectionTest, BasicSpellAvailable)
{
    EXPECT_TRUE(CanCastSpell(1));  // Sinister Strike
}

TEST_F(SpellSelectionTest, OnCooldownNotAvailable)
{
    spellStates_[1].cooldownRemaining = 5000;
    EXPECT_FALSE(CanCastSpell(1));
}

TEST_F(SpellSelectionTest, NotEnoughMana)
{
    context_.currentMana = 30.0f;
    EXPECT_FALSE(CanCastSpell(1));  // Costs 45
}

TEST_F(SpellSelectionTest, OutOfRange)
{
    context_.targetDistance = 10.0f;
    EXPECT_FALSE(CanCastSpell(1));  // Max range 5
}

TEST_F(SpellSelectionTest, RequiresBehind)
{
    EXPECT_FALSE(CanCastSpell(2));  // Backstab requires behind

    context_.isBehindTarget = true;
    EXPECT_TRUE(CanCastSpell(2));
}

TEST_F(SpellSelectionTest, ComboPointRequirement)
{
    EXPECT_FALSE(CanCastSpell(3));  // Eviscerate needs 5 CP

    context_.comboPoints = 5;
    EXPECT_TRUE(CanCastSpell(3));
}

/**
 * @brief Tests for rotation priority
 */
class RotationPriorityTest : public ::testing::Test
{
protected:
    struct RotationSpell
    {
        uint32 id;
        std::string name;
        float basePriority;
        bool isFinisher;
        bool isBuilder;
        bool isCooldown;
        bool isMaintenance;  // DoT/Buff that needs to be kept up
    };

    struct RotationContext
    {
        int comboPoints;
        bool hasMaintenanceBuff;
        bool targetHasDot;
        float targetHealthPercent;
        bool cooldownsActive;
    };

    std::vector<RotationSpell> rotation_;
    RotationContext context_;

    void SetUp() override
    {
        rotation_ = {
            {1, "Slice and Dice", 100.0f, false, false, false, true},
            {2, "Rupture", 90.0f, true, false, false, true},
            {3, "Eviscerate", 80.0f, true, false, false, false},
            {4, "Mutilate", 50.0f, false, true, false, false},
            {5, "Cold Blood", 200.0f, false, false, true, false},
        };

        context_ = {0, true, true, 100.0f, false};
    }

    float CalculateSpellPriority(RotationSpell const& spell)
    {
        float priority = spell.basePriority;

        // Maintenance buffs highest priority when missing
        if (spell.isMaintenance)
        {
            if (spell.name == "Slice and Dice" && !context_.hasMaintenanceBuff)
                priority += 150.0f;
            if (spell.name == "Rupture" && !context_.targetHasDot)
                priority += 100.0f;
        }

        // Finishers at max combo points
        if (spell.isFinisher && context_.comboPoints >= 5)
            priority += 50.0f;

        // Builders when low on combo points
        if (spell.isBuilder && context_.comboPoints < 5)
            priority += 30.0f;

        // Cooldowns during burn phases
        if (spell.isCooldown && !context_.cooldownsActive)
            priority += 100.0f;

        // Execute phase bonus for damage finishers
        if (spell.name == "Eviscerate" && context_.targetHealthPercent <= 35.0f)
            priority += 50.0f;

        return priority;
    }

    std::string GetNextSpell()
    {
        RotationSpell const* best = nullptr;
        float bestPriority = -1.0f;

        for (auto const& spell : rotation_)
        {
            float priority = CalculateSpellPriority(spell);
            if (priority > bestPriority)
            {
                best = &spell;
                bestPriority = priority;
            }
        }

        return best ? best->name : "";
    }
};

TEST_F(RotationPriorityTest, MissingBuffHighPriority)
{
    context_.hasMaintenanceBuff = false;
    context_.comboPoints = 5;
    EXPECT_EQ("Slice and Dice", GetNextSpell());
}

TEST_F(RotationPriorityTest, MissingDotHighPriority)
{
    context_.targetHasDot = false;
    context_.comboPoints = 5;
    EXPECT_EQ("Rupture", GetNextSpell());
}

TEST_F(RotationPriorityTest, CooldownsWhenAvailable)
{
    context_.cooldownsActive = false;
    EXPECT_EQ("Cold Blood", GetNextSpell());
}

TEST_F(RotationPriorityTest, BuilderWhenLowComboPoints)
{
    context_.cooldownsActive = true;  // No CD priority
    context_.comboPoints = 2;
    EXPECT_EQ("Mutilate", GetNextSpell());
}

TEST_F(RotationPriorityTest, FinisherAtMaxComboPoints)
{
    context_.cooldownsActive = true;
    context_.comboPoints = 5;
    EXPECT_EQ("Eviscerate", GetNextSpell());
}

TEST_F(RotationPriorityTest, ExecutePhaseBonus)
{
    context_.cooldownsActive = true;
    context_.comboPoints = 5;
    context_.targetHealthPercent = 20.0f;
    // Eviscerate gets execute bonus
    EXPECT_EQ("Eviscerate", GetNextSpell());
}

/**
 * @brief Tests for GCD and cast timing
 */
class SpellTimingTest : public ::testing::Test
{
protected:
    struct SpellTiming
    {
        uint32 gcdDuration;
        uint32 castTime;
        uint32 channelDuration;
        bool isInstant;
    };

    struct TimingState
    {
        uint32 gcdEndTime;
        uint32 castEndTime;
        uint32 channelEndTime;
    };

    TimingState state_;

    void SetUp() override
    {
        state_ = {0, 0, 0};
    }

    bool CanStartCast(uint32 currentTime)
    {
        if (currentTime < state_.gcdEndTime)
            return false;
        if (currentTime < state_.castEndTime)
            return false;
        if (currentTime < state_.channelEndTime)
            return false;
        return true;
    }

    void StartCast(uint32 currentTime, SpellTiming const& spell)
    {
        state_.gcdEndTime = currentTime + spell.gcdDuration;

        if (spell.channelDuration > 0)
            state_.channelEndTime = currentTime + spell.channelDuration;
        else if (!spell.isInstant)
            state_.castEndTime = currentTime + spell.castTime;
    }

    uint32 GetNextActionTime()
    {
        return std::max({state_.gcdEndTime, state_.castEndTime, state_.channelEndTime});
    }
};

TEST_F(SpellTimingTest, CanCastInitially)
{
    EXPECT_TRUE(CanStartCast(0));
}

TEST_F(SpellTimingTest, GcdPreventsNextCast)
{
    SpellTiming instant{1500, 0, 0, true};
    StartCast(0, instant);

    EXPECT_FALSE(CanStartCast(500));
    EXPECT_FALSE(CanStartCast(1000));
    EXPECT_TRUE(CanStartCast(1500));
}

TEST_F(SpellTimingTest, CastTimePreventsNextCast)
{
    SpellTiming hardCast{1500, 2500, 0, false};
    StartCast(0, hardCast);

    EXPECT_FALSE(CanStartCast(2000));  // Still casting
    EXPECT_TRUE(CanStartCast(2500));   // Cast finished
}

TEST_F(SpellTimingTest, ChannelPreventsNextCast)
{
    SpellTiming channel{1500, 0, 3000, false};
    StartCast(0, channel);

    EXPECT_FALSE(CanStartCast(2000));  // Still channeling
    EXPECT_TRUE(CanStartCast(3000));   // Channel finished
}

TEST_F(SpellTimingTest, NextActionTime)
{
    SpellTiming hardCast{1500, 2500, 0, false};
    StartCast(0, hardCast);

    EXPECT_EQ(2500u, GetNextActionTime());  // Cast > GCD
}

/**
 * @brief Tests for proc-based rotation adjustments
 */
class ProcHandlingTest : public ::testing::Test
{
protected:
    struct Proc
    {
        std::string name;
        uint32 spellToUse;
        float priorityBonus;
        uint32 duration;
        uint32 activatedAt;
    };

    std::vector<Proc> activeProcs_;

    void ActivateProc(std::string const& name, uint32 spellId, float bonus, uint32 duration, uint32 currentTime)
    {
        activeProcs_.push_back({name, spellId, bonus, duration, currentTime});
    }

    void ExpireProcs(uint32 currentTime)
    {
        activeProcs_.erase(
            std::remove_if(activeProcs_.begin(), activeProcs_.end(),
                [currentTime](Proc const& p) {
                    return currentTime >= p.activatedAt + p.duration;
                }),
            activeProcs_.end());
    }

    bool HasProc(std::string const& name)
    {
        return std::any_of(activeProcs_.begin(), activeProcs_.end(),
            [&name](Proc const& p) { return p.name == name; });
    }

    float GetProcBonusForSpell(uint32 spellId)
    {
        float bonus = 0.0f;
        for (auto const& proc : activeProcs_)
        {
            if (proc.spellToUse == spellId)
                bonus += proc.priorityBonus;
        }
        return bonus;
    }
};

TEST_F(ProcHandlingTest, ProcActivates)
{
    ActivateProc("Hot Streak", 1, 100.0f, 5000, 0);
    EXPECT_TRUE(HasProc("Hot Streak"));
}

TEST_F(ProcHandlingTest, ProcExpires)
{
    ActivateProc("Hot Streak", 1, 100.0f, 5000, 0);
    ExpireProcs(6000);
    EXPECT_FALSE(HasProc("Hot Streak"));
}

TEST_F(ProcHandlingTest, ProcBoostsPriority)
{
    ActivateProc("Hot Streak", 1, 100.0f, 5000, 0);
    EXPECT_FLOAT_EQ(100.0f, GetProcBonusForSpell(1));
    EXPECT_FLOAT_EQ(0.0f, GetProcBonusForSpell(2));
}

TEST_F(ProcHandlingTest, MultipleProcsStack)
{
    ActivateProc("Proc1", 1, 50.0f, 5000, 0);
    ActivateProc("Proc2", 1, 75.0f, 5000, 0);
    EXPECT_FLOAT_EQ(125.0f, GetProcBonusForSpell(1));
}

