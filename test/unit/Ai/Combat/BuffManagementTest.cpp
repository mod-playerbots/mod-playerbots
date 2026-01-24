/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

/**
 * @brief Tests for buff management logic
 */

/**
 * @brief Tests for self-buff management
 */
class SelfBuffTest : public ::testing::Test
{
protected:
    struct Buff
    {
        uint32 spellId;
        std::string name;
        uint32 durationMs;
        uint32 remainingMs;
        bool isActive;
    };

    struct BuffContext
    {
        std::vector<Buff> requiredBuffs;
        bool inCombat;
        bool isMounted;
        float manaPercent;
    };

    BuffContext context_;

    void SetUp() override
    {
        context_.requiredBuffs = {
            {1, "Arcane Intellect", 1800000, 0, false},
            {2, "Mage Armor", 1800000, 0, false},
            {3, "Ice Barrier", 60000, 0, false},
        };
        context_.inCombat = false;
        context_.isMounted = false;
        context_.manaPercent = 100.0f;
    }

    bool NeedsRefresh(Buff const& buff, uint32 refreshThreshold = 300000)
    {
        if (!buff.isActive)
            return true;
        return buff.remainingMs < refreshThreshold;
    }

    std::vector<uint32> GetMissingBuffs()
    {
        std::vector<uint32> missing;
        for (auto const& buff : context_.requiredBuffs)
        {
            if (NeedsRefresh(buff))
                missing.push_back(buff.spellId);
        }
        return missing;
    }

    bool ShouldBuffNow()
    {
        if (context_.isMounted)
            return false;

        // Don't buff with low mana unless out of combat
        if (context_.manaPercent < 30.0f && context_.inCombat)
            return false;

        return !GetMissingBuffs().empty();
    }

    uint32 GetNextBuffToApply()
    {
        auto missing = GetMissingBuffs();
        if (missing.empty())
            return 0;

        // Priority: shortest duration buffs first (they need more upkeep)
        uint32 bestSpell = 0;
        uint32 shortestDuration = UINT32_MAX;

        for (uint32 spellId : missing)
        {
            for (auto const& buff : context_.requiredBuffs)
            {
                if (buff.spellId == spellId && buff.durationMs < shortestDuration)
                {
                    shortestDuration = buff.durationMs;
                    bestSpell = spellId;
                }
            }
        }

        return bestSpell;
    }
};

TEST_F(SelfBuffTest, DetectsMissingBuffs)
{
    auto missing = GetMissingBuffs();
    EXPECT_EQ(3u, missing.size());
}

TEST_F(SelfBuffTest, DetectsExpiringBuffs)
{
    context_.requiredBuffs[0].isActive = true;
    context_.requiredBuffs[0].remainingMs = 200000;  // Below threshold
    context_.requiredBuffs[1].isActive = true;
    context_.requiredBuffs[1].remainingMs = 1000000;  // Above threshold

    auto missing = GetMissingBuffs();
    EXPECT_EQ(2u, missing.size());  // First and third
}

TEST_F(SelfBuffTest, DoesNotBuffWhenMounted)
{
    context_.isMounted = true;
    EXPECT_FALSE(ShouldBuffNow());
}

TEST_F(SelfBuffTest, DoesNotBuffWithLowManaInCombat)
{
    context_.inCombat = true;
    context_.manaPercent = 20.0f;
    EXPECT_FALSE(ShouldBuffNow());
}

TEST_F(SelfBuffTest, BuffsWithLowManaOutOfCombat)
{
    context_.inCombat = false;
    context_.manaPercent = 20.0f;
    EXPECT_TRUE(ShouldBuffNow());
}

TEST_F(SelfBuffTest, PrioritizesShortDurationBuffs)
{
    uint32 next = GetNextBuffToApply();
    EXPECT_EQ(3u, next);  // Ice Barrier has 60s duration
}

/**
 * @brief Tests for group buff management
 */
class GroupBuffTest : public ::testing::Test
{
protected:
    struct GroupMember
    {
        uint32 guid;
        std::string playerClass;
        bool hasBuff;
        float distance;
        bool isAlive;
    };

    struct GroupBuffInfo
    {
        uint32 singleSpellId;
        uint32 groupSpellId;
        std::set<std::string> validClasses;
        uint32 singleManaCost;
        uint32 groupManaCost;
    };

    std::vector<GroupMember> group_;
    GroupBuffInfo buffInfo_;

    void SetUp() override
    {
        group_ = {
            {1, "Warrior", false, 10.0f, true},
            {2, "Mage", false, 15.0f, true},
            {3, "Priest", false, 20.0f, true},
            {4, "Rogue", false, 25.0f, true},
            {5, "Druid", false, 30.0f, true},
        };

        // Arcane Intellect example
        buffInfo_ = {
            1,      // Single target spell
            2,      // Group spell
            {"Mage", "Priest", "Warlock", "Druid", "Paladin", "Shaman"},
            500,    // Single mana cost
            2000,   // Group mana cost
        };
    }

    std::vector<uint32> GetMembersNeedingBuff()
    {
        std::vector<uint32> needing;
        for (auto const& member : group_)
        {
            if (!member.isAlive)
                continue;
            if (member.hasBuff)
                continue;
            if (buffInfo_.validClasses.find(member.playerClass) == buffInfo_.validClasses.end())
                continue;
            needing.push_back(member.guid);
        }
        return needing;
    }

    bool ShouldUseGroupBuff(float manaPercent)
    {
        auto needing = GetMembersNeedingBuff();

        if (needing.size() < 3)
            return false;  // Not worth group mana cost

        // Check if we have enough mana
        return manaPercent >= 40.0f;
    }

    uint32 GetClosestMemberNeedingBuff(float maxRange)
    {
        uint32 closest = 0;
        float closestDist = maxRange + 1.0f;

        for (auto const& member : group_)
        {
            if (!member.isAlive || member.hasBuff)
                continue;
            if (buffInfo_.validClasses.find(member.playerClass) == buffInfo_.validClasses.end())
                continue;
            if (member.distance > maxRange)
                continue;

            if (member.distance < closestDist)
            {
                closestDist = member.distance;
                closest = member.guid;
            }
        }

        return closest;
    }
};

TEST_F(GroupBuffTest, IdentifiesMembersNeedingBuff)
{
    auto needing = GetMembersNeedingBuff();
    // Mage, Priest, Druid can receive Arcane Intellect
    EXPECT_EQ(3u, needing.size());
}

TEST_F(GroupBuffTest, ExcludesDeadMembers)
{
    group_[1].isAlive = false;  // Mage dead
    auto needing = GetMembersNeedingBuff();
    EXPECT_EQ(2u, needing.size());
}

TEST_F(GroupBuffTest, ExcludesAlreadyBuffed)
{
    group_[1].hasBuff = true;  // Mage already has buff
    auto needing = GetMembersNeedingBuff();
    EXPECT_EQ(2u, needing.size());
}

TEST_F(GroupBuffTest, UseGroupBuffWhenManyNeed)
{
    EXPECT_TRUE(ShouldUseGroupBuff(100.0f));
}

TEST_F(GroupBuffTest, DontUseGroupBuffWhenFewNeed)
{
    group_[1].hasBuff = true;
    group_[2].hasBuff = true;
    EXPECT_FALSE(ShouldUseGroupBuff(100.0f));
}

TEST_F(GroupBuffTest, DontUseGroupBuffWithLowMana)
{
    EXPECT_FALSE(ShouldUseGroupBuff(20.0f));
}

TEST_F(GroupBuffTest, FindsClosestMember)
{
    uint32 closest = GetClosestMemberNeedingBuff(40.0f);
    EXPECT_EQ(2u, closest);  // Mage at 15.0f is closest valid target
}

/**
 * @brief Tests for buff priority and stacking
 */
class BuffPriorityTest : public ::testing::Test
{
protected:
    struct BuffSlot
    {
        uint32 spellId;
        std::string category;  // "food", "flask", "elixir_battle", "elixir_guardian"
        float effectValue;
        bool isActive;
    };

    std::map<std::string, int> categoryLimits_;
    std::vector<BuffSlot> buffs_;

    void SetUp() override
    {
        categoryLimits_ = {
            {"food", 1},
            {"flask", 1},
            {"elixir_battle", 1},
            {"elixir_guardian", 1},
        };

        buffs_ = {
            {1, "food", 40.0f, false},      // Food buff
            {2, "flask", 125.0f, false},    // Flask
            {3, "elixir_battle", 60.0f, false},   // Battle elixir
            {4, "elixir_guardian", 50.0f, false}, // Guardian elixir
        };
    }

    int GetActiveCountInCategory(std::string const& category)
    {
        int count = 0;
        for (auto const& buff : buffs_)
        {
            if (buff.category == category && buff.isActive)
                count++;
        }
        return count;
    }

    bool CanApplyBuff(std::string const& category)
    {
        auto limitIt = categoryLimits_.find(category);
        if (limitIt == categoryLimits_.end())
            return true;

        return GetActiveCountInCategory(category) < limitIt->second;
    }

    bool FlaskExcludesElixirs()
    {
        // When flask is active, can't use battle/guardian elixirs
        for (auto const& buff : buffs_)
        {
            if (buff.category == "flask" && buff.isActive)
                return true;
        }
        return false;
    }

    bool CanUseElixir()
    {
        return !FlaskExcludesElixirs();
    }

    std::vector<uint32> GetOptimalConsumables(bool hasFlask)
    {
        std::vector<uint32> result;

        if (hasFlask)
        {
            // Use flask
            result.push_back(2);
        }
        else
        {
            // Use both elixirs
            result.push_back(3);
            result.push_back(4);
        }

        // Always use food
        result.push_back(1);

        return result;
    }
};

TEST_F(BuffPriorityTest, CategoryLimitEnforced)
{
    EXPECT_TRUE(CanApplyBuff("food"));

    buffs_[0].isActive = true;
    EXPECT_FALSE(CanApplyBuff("food"));
}

TEST_F(BuffPriorityTest, FlaskBlocksElixirs)
{
    EXPECT_TRUE(CanUseElixir());

    buffs_[1].isActive = true;  // Flask active
    EXPECT_FALSE(CanUseElixir());
}

TEST_F(BuffPriorityTest, OptimalWithFlask)
{
    auto optimal = GetOptimalConsumables(true);
    EXPECT_EQ(2u, optimal.size());  // Flask + Food

    bool hasFlask = std::find(optimal.begin(), optimal.end(), 2) != optimal.end();
    bool hasFood = std::find(optimal.begin(), optimal.end(), 1) != optimal.end();
    EXPECT_TRUE(hasFlask);
    EXPECT_TRUE(hasFood);
}

TEST_F(BuffPriorityTest, OptimalWithoutFlask)
{
    auto optimal = GetOptimalConsumables(false);
    EXPECT_EQ(3u, optimal.size());  // Both elixirs + Food

    bool hasBattle = std::find(optimal.begin(), optimal.end(), 3) != optimal.end();
    bool hasGuardian = std::find(optimal.begin(), optimal.end(), 4) != optimal.end();
    bool hasFood = std::find(optimal.begin(), optimal.end(), 1) != optimal.end();
    EXPECT_TRUE(hasBattle);
    EXPECT_TRUE(hasGuardian);
    EXPECT_TRUE(hasFood);
}

/**
 * @brief Tests for debuff tracking
 */
class DebuffTrackingTest : public ::testing::Test
{
protected:
    struct Debuff
    {
        uint32 spellId;
        std::string name;
        uint32 remainingMs;
        uint32 stackCount;
        uint32 maxStacks;
        bool isDispellable;
        std::string dispelType;  // "magic", "poison", "disease", "curse"
    };

    struct DebuffContext
    {
        std::vector<Debuff> debuffs;
        bool canDispelMagic;
        bool canDispelPoison;
        bool canDispelDisease;
        bool canDispelCurse;
    };

    DebuffContext context_;

    void SetUp() override
    {
        context_.debuffs = {
            {1, "Curse of Agony", 20000, 1, 1, true, "curse"},
            {2, "Corruption", 15000, 1, 1, true, "magic"},
            {3, "Deadly Poison", 12000, 3, 5, true, "poison"},
        };
        context_.canDispelMagic = true;
        context_.canDispelPoison = false;
        context_.canDispelDisease = false;
        context_.canDispelCurse = true;
    }

    bool CanDispel(Debuff const& debuff)
    {
        if (!debuff.isDispellable)
            return false;

        if (debuff.dispelType == "magic")
            return context_.canDispelMagic;
        if (debuff.dispelType == "poison")
            return context_.canDispelPoison;
        if (debuff.dispelType == "disease")
            return context_.canDispelDisease;
        if (debuff.dispelType == "curse")
            return context_.canDispelCurse;

        return false;
    }

    std::vector<uint32> GetDispellableDebuffs()
    {
        std::vector<uint32> result;
        for (auto const& debuff : context_.debuffs)
        {
            if (CanDispel(debuff))
                result.push_back(debuff.spellId);
        }
        return result;
    }

    uint32 GetHighestPriorityDispel()
    {
        Debuff const* best = nullptr;
        float bestScore = -1.0f;

        for (auto const& debuff : context_.debuffs)
        {
            if (!CanDispel(debuff))
                continue;

            // Priority based on remaining time (shorter = more urgent)
            // and stack count (more stacks = more urgent)
            float score = (1.0f / std::max(1u, debuff.remainingMs)) * 1000000.0f;
            score += debuff.stackCount * 10.0f;

            if (score > bestScore)
            {
                best = &debuff;
                bestScore = score;
            }
        }

        return best ? best->spellId : 0;
    }
};

TEST_F(DebuffTrackingTest, IdentifiesDispellable)
{
    auto dispellable = GetDispellableDebuffs();
    EXPECT_EQ(2u, dispellable.size());  // Curse and Magic, not Poison
}

TEST_F(DebuffTrackingTest, CannotDispelWhenMissingAbility)
{
    context_.canDispelCurse = false;
    auto dispellable = GetDispellableDebuffs();
    EXPECT_EQ(1u, dispellable.size());  // Only Magic
}

TEST_F(DebuffTrackingTest, PrioritizesUrgentDebuffs)
{
    // Corruption has shortest duration
    uint32 priority = GetHighestPriorityDispel();
    EXPECT_EQ(2u, priority);
}

