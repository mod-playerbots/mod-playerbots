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
 * @brief Tests for resource management logic
 */

/**
 * @brief Tests for potion usage
 */
class PotionUsageTest : public ::testing::Test
{
protected:
    struct Potion
    {
        uint32 itemId;
        std::string type;  // "health", "mana", "both"
        uint32 restoreAmount;
        uint32 cooldownMs;
    };

    struct PotionContext
    {
        float healthPercent;
        float manaPercent;
        bool inCombat;
        uint32 potionCooldownRemaining;
        std::vector<Potion> availablePotions;
    };

    PotionContext context_;

    void SetUp() override
    {
        context_ = {
            100.0f, 100.0f, true, 0,
            {
                {1, "health", 2000, 60000},
                {2, "mana", 3000, 60000},
                {3, "both", 1500, 60000},  // Rejuvenation potion
            }
        };
    }

    bool ShouldUseHealthPotion()
    {
        if (context_.potionCooldownRemaining > 0)
            return false;
        if (!context_.inCombat)
            return false;
        return context_.healthPercent < 35.0f;
    }

    bool ShouldUseManaPotion()
    {
        if (context_.potionCooldownRemaining > 0)
            return false;
        if (!context_.inCombat)
            return false;
        return context_.manaPercent < 20.0f;
    }

    uint32 SelectBestPotion()
    {
        if (context_.potionCooldownRemaining > 0)
            return 0;

        bool needHealth = context_.healthPercent < 35.0f;
        bool needMana = context_.manaPercent < 20.0f;

        if (needHealth && needMana)
        {
            // Look for combo potion first
            for (auto const& potion : context_.availablePotions)
            {
                if (potion.type == "both")
                    return potion.itemId;
            }
        }

        if (needHealth)
        {
            for (auto const& potion : context_.availablePotions)
            {
                if (potion.type == "health")
                    return potion.itemId;
            }
        }

        if (needMana)
        {
            for (auto const& potion : context_.availablePotions)
            {
                if (potion.type == "mana")
                    return potion.itemId;
            }
        }

        return 0;
    }
};

TEST_F(PotionUsageTest, NoNeedAtFullHealth)
{
    EXPECT_FALSE(ShouldUseHealthPotion());
}

TEST_F(PotionUsageTest, UseHealthPotionWhenLow)
{
    context_.healthPercent = 25.0f;
    EXPECT_TRUE(ShouldUseHealthPotion());
}

TEST_F(PotionUsageTest, NoPotionOutOfCombat)
{
    context_.healthPercent = 25.0f;
    context_.inCombat = false;
    EXPECT_FALSE(ShouldUseHealthPotion());
}

TEST_F(PotionUsageTest, NoPotionOnCooldown)
{
    context_.healthPercent = 25.0f;
    context_.potionCooldownRemaining = 30000;
    EXPECT_FALSE(ShouldUseHealthPotion());
}

TEST_F(PotionUsageTest, SelectsHealthPotion)
{
    context_.healthPercent = 25.0f;
    EXPECT_EQ(1u, SelectBestPotion());
}

TEST_F(PotionUsageTest, SelectsManaPotion)
{
    context_.manaPercent = 15.0f;
    EXPECT_EQ(2u, SelectBestPotion());
}

TEST_F(PotionUsageTest, SelectsComboPotionWhenBothNeeded)
{
    context_.healthPercent = 25.0f;
    context_.manaPercent = 15.0f;
    EXPECT_EQ(3u, SelectBestPotion());
}

/**
 * @brief Tests for healthstone/mana gem usage
 */
class SpecialItemUsageTest : public ::testing::Test
{
protected:
    struct SpecialItem
    {
        uint32 itemId;
        std::string type;
        uint32 restoreAmount;
        uint32 cooldownMs;
        uint32 cooldownRemaining;
    };

    struct ItemContext
    {
        float healthPercent;
        float manaPercent;
        bool inCombat;
        uint32 potionCooldownRemaining;
        std::vector<SpecialItem> items;
    };

    ItemContext context_;

    void SetUp() override
    {
        context_ = {
            100.0f, 100.0f, true, 0,
            {
                {1, "healthstone", 4000, 120000, 0},
                {2, "mana_gem", 3000, 120000, 0},
            }
        };
    }

    bool ShouldUseHealthstone()
    {
        for (auto const& item : context_.items)
        {
            if (item.type == "healthstone" && item.cooldownRemaining == 0)
            {
                // Use at lower threshold than potion (save for emergencies)
                return context_.healthPercent < 25.0f;
            }
        }
        return false;
    }

    bool ShouldUseManaGem()
    {
        for (auto const& item : context_.items)
        {
            if (item.type == "mana_gem" && item.cooldownRemaining == 0)
            {
                return context_.manaPercent < 15.0f;
            }
        }
        return false;
    }

    bool PrefersHealthstoneOverPotion()
    {
        // Healthstone doesn't share potion cooldown, so prefer it
        // to save potion for when we need mana too
        for (auto const& item : context_.items)
        {
            if (item.type == "healthstone" && item.cooldownRemaining == 0)
                return true;
        }
        return false;
    }
};

TEST_F(SpecialItemUsageTest, UseHealthstoneWhenCritical)
{
    context_.healthPercent = 20.0f;
    EXPECT_TRUE(ShouldUseHealthstone());
}

TEST_F(SpecialItemUsageTest, DontUseHealthstoneWhenNotCritical)
{
    context_.healthPercent = 30.0f;  // Above 25% threshold
    EXPECT_FALSE(ShouldUseHealthstone());
}

TEST_F(SpecialItemUsageTest, UseManaGemWhenLow)
{
    context_.manaPercent = 10.0f;
    EXPECT_TRUE(ShouldUseManaGem());
}

TEST_F(SpecialItemUsageTest, PreferHealthstoneOverPotion)
{
    EXPECT_TRUE(PrefersHealthstoneOverPotion());
}

/**
 * @brief Tests for mana conservation
 */
class ManaConservationTest : public ::testing::Test
{
protected:
    struct Spell
    {
        uint32 id;
        std::string name;
        uint32 manaCost;
        float efficiency;  // Effect per mana
        bool isExpensive;
    };

    struct ConservationContext
    {
        float manaPercent;
        bool inCombat;
        float combatTimeRemaining;  // Estimated seconds
        std::vector<Spell> spells;
    };

    ConservationContext context_;

    void SetUp() override
    {
        context_ = {
            100.0f, true, 60.0f,
            {
                {1, "Flash Heal", 400, 2.0f, true},
                {2, "Heal", 300, 3.0f, false},
                {3, "Greater Heal", 700, 2.5f, true},
            }
        };
    }

    float GetManaConservationMultiplier()
    {
        if (context_.manaPercent > 80.0f)
            return 1.0f;  // No conservation needed
        if (context_.manaPercent > 50.0f)
            return 1.2f;  // Slight preference for efficient spells
        if (context_.manaPercent > 30.0f)
            return 1.5f;  // Strong preference
        return 2.0f;      // Critical - heavily favor efficient spells
    }

    float CalculateSpellScore(Spell const& spell)
    {
        float baseScore = spell.efficiency * 100.0f;

        // Apply conservation multiplier
        float conservationMult = GetManaConservationMultiplier();
        if (spell.isExpensive)
            baseScore /= conservationMult;

        return baseScore;
    }

    uint32 SelectMostEfficientSpell()
    {
        Spell const* best = nullptr;
        float bestScore = -1.0f;

        for (auto const& spell : context_.spells)
        {
            float score = CalculateSpellScore(spell);
            if (score > bestScore)
            {
                best = &spell;
                bestScore = score;
            }
        }

        return best ? best->id : 0;
    }

    bool ShouldWandInsteadOfCast()
    {
        // Wand when mana is very low and not in emergency
        return context_.manaPercent < 10.0f;
    }
};

TEST_F(ManaConservationTest, NoConservationAtHighMana)
{
    EXPECT_FLOAT_EQ(1.0f, GetManaConservationMultiplier());
}

TEST_F(ManaConservationTest, ModerateConservationAtMediumMana)
{
    context_.manaPercent = 60.0f;
    EXPECT_FLOAT_EQ(1.2f, GetManaConservationMultiplier());
}

TEST_F(ManaConservationTest, StrongConservationAtLowMana)
{
    context_.manaPercent = 40.0f;
    EXPECT_FLOAT_EQ(1.5f, GetManaConservationMultiplier());
}

TEST_F(ManaConservationTest, CriticalConservationAtVeryLowMana)
{
    context_.manaPercent = 20.0f;
    EXPECT_FLOAT_EQ(2.0f, GetManaConservationMultiplier());
}

TEST_F(ManaConservationTest, SelectsEfficientSpellWhenConserving)
{
    context_.manaPercent = 30.0f;
    uint32 selected = SelectMostEfficientSpell();
    EXPECT_EQ(2u, selected);  // Heal is most efficient
}

TEST_F(ManaConservationTest, WandAtCriticalMana)
{
    context_.manaPercent = 5.0f;
    EXPECT_TRUE(ShouldWandInsteadOfCast());
}

/**
 * @brief Tests for food/drink usage
 */
class FoodDrinkUsageTest : public ::testing::Test
{
protected:
    struct Consumable
    {
        uint32 itemId;
        std::string type;  // "food", "drink", "both"
        uint32 restorePerTick;
        uint32 totalRestore;
    };

    struct RestContext
    {
        float healthPercent;
        float manaPercent;
        bool inCombat;
        bool isSitting;
        bool isEating;
        bool isDrinking;
        std::vector<Consumable> available;
    };

    RestContext context_;

    void SetUp() override
    {
        context_ = {
            50.0f, 50.0f, false, false, false, false,
            {
                {1, "food", 250, 7500},
                {2, "drink", 300, 9000},
                {3, "both", 200, 6000},
            }
        };
    }

    bool ShouldEat()
    {
        if (context_.inCombat)
            return false;
        if (context_.isEating)
            return false;
        return context_.healthPercent < 80.0f;
    }

    bool ShouldDrink()
    {
        if (context_.inCombat)
            return false;
        if (context_.isDrinking)
            return false;
        return context_.manaPercent < 80.0f;
    }

    bool ShouldUseMageTable()
    {
        // Use mage table food/drink if available and need both
        bool needHealth = context_.healthPercent < 80.0f;
        bool needMana = context_.manaPercent < 80.0f;

        if (needHealth && needMana)
        {
            for (auto const& item : context_.available)
            {
                if (item.type == "both")
                    return true;
            }
        }
        return false;
    }

    bool IsReadyToContinue()
    {
        return context_.healthPercent >= 80.0f && context_.manaPercent >= 80.0f;
    }

    float EstimateTimeToFull(float currentPercent, uint32 restorePerTick, float maxValue)
    {
        float deficit = (100.0f - currentPercent) / 100.0f * maxValue;
        float ticksNeeded = deficit / restorePerTick;
        return ticksNeeded * 2.0f;  // Assume 2 second ticks
    }
};

TEST_F(FoodDrinkUsageTest, EatWhenHealthLow)
{
    EXPECT_TRUE(ShouldEat());
}

TEST_F(FoodDrinkUsageTest, DontEatInCombat)
{
    context_.inCombat = true;
    EXPECT_FALSE(ShouldEat());
}

TEST_F(FoodDrinkUsageTest, DontEatWhenAlreadyEating)
{
    context_.isEating = true;
    EXPECT_FALSE(ShouldEat());
}

TEST_F(FoodDrinkUsageTest, DontEatWhenHealthy)
{
    context_.healthPercent = 90.0f;
    EXPECT_FALSE(ShouldEat());
}

TEST_F(FoodDrinkUsageTest, DrinkWhenManaLow)
{
    EXPECT_TRUE(ShouldDrink());
}

TEST_F(FoodDrinkUsageTest, UseMageTableWhenBothNeeded)
{
    EXPECT_TRUE(ShouldUseMageTable());
}

TEST_F(FoodDrinkUsageTest, DontUseMageTableWhenOnlyOneNeeded)
{
    context_.healthPercent = 90.0f;
    EXPECT_FALSE(ShouldUseMageTable());
}

TEST_F(FoodDrinkUsageTest, ReadyToContinue)
{
    EXPECT_FALSE(IsReadyToContinue());

    context_.healthPercent = 90.0f;
    context_.manaPercent = 85.0f;
    EXPECT_TRUE(IsReadyToContinue());
}

TEST_F(FoodDrinkUsageTest, EstimatesTimeToFull)
{
    float time = EstimateTimeToFull(50.0f, 250, 10000.0f);
    // 50% of 10000 = 5000 deficit, 5000/250 = 20 ticks, 20*2 = 40 seconds
    EXPECT_NEAR(40.0f, time, 0.1f);
}

