/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <cmath>

/**
 * @brief Tests for pet management logic
 */

/**
 * @brief Tests for pet stance management
 */
class PetStanceTest : public ::testing::Test
{
protected:
    enum class PetStance
    {
        Passive,
        Defensive,
        Aggressive
    };

    struct PetContext
    {
        bool ownerInCombat;
        bool petHasTarget;
        float petHealthPercent;
        float distanceToOwner;
        bool isPulling;  // Owner is pulling mobs
    };

    PetContext context_;
    PetStance currentStance_;

    void SetUp() override
    {
        context_ = {false, false, 100.0f, 5.0f, false};
        currentStance_ = PetStance::Defensive;
    }

    PetStance DetermineOptimalStance()
    {
        // During pulls, be passive to not break CC
        if (context_.isPulling)
            return PetStance::Passive;

        // Low health pet should be passive
        if (context_.petHealthPercent < 20.0f)
            return PetStance::Passive;

        // In combat, be defensive
        if (context_.ownerInCombat)
            return PetStance::Defensive;

        // Out of combat, stay passive
        return PetStance::Passive;
    }

    bool ShouldRecallPet()
    {
        // Recall if too far
        if (context_.distanceToOwner > 40.0f)
            return true;

        // Recall if pet health is critical
        if (context_.petHealthPercent < 15.0f)
            return true;

        return false;
    }
};

TEST_F(PetStanceTest, PassiveDuringPull)
{
    context_.isPulling = true;
    EXPECT_EQ(PetStance::Passive, DetermineOptimalStance());
}

TEST_F(PetStanceTest, PassiveWhenLowHealth)
{
    context_.petHealthPercent = 15.0f;
    EXPECT_EQ(PetStance::Passive, DetermineOptimalStance());
}

TEST_F(PetStanceTest, DefensiveInCombat)
{
    context_.ownerInCombat = true;
    EXPECT_EQ(PetStance::Defensive, DetermineOptimalStance());
}

TEST_F(PetStanceTest, PassiveOutOfCombat)
{
    EXPECT_EQ(PetStance::Passive, DetermineOptimalStance());
}

TEST_F(PetStanceTest, RecallWhenFar)
{
    context_.distanceToOwner = 50.0f;
    EXPECT_TRUE(ShouldRecallPet());
}

TEST_F(PetStanceTest, RecallWhenCriticalHealth)
{
    context_.petHealthPercent = 10.0f;
    EXPECT_TRUE(ShouldRecallPet());
}

/**
 * @brief Tests for pet ability usage
 */
class PetAbilityTest : public ::testing::Test
{
protected:
    struct PetAbility
    {
        uint32 id;
        std::string name;
        std::string type;  // "damage", "utility", "defensive"
        uint32 cooldownMs;
        uint32 cooldownRemaining;
        bool autocast;
    };

    struct PetAbilityContext
    {
        bool inCombat;
        bool petHasTarget;
        float petFocusPercent;
        std::vector<PetAbility> abilities;
    };

    PetAbilityContext context_;

    void SetUp() override
    {
        context_ = {
            true, true, 100.0f,
            {
                {1, "Bite", "damage", 0, 0, true},
                {2, "Claw", "damage", 0, 0, true},
                {3, "Growl", "utility", 5000, 0, true},  // Taunt
                {4, "Cower", "defensive", 10000, 0, false},
                {5, "Dash", "utility", 30000, 0, false},
            }
        };
    }

    std::vector<uint32> GetAutocastAbilities()
    {
        std::vector<uint32> result;
        for (auto const& ability : context_.abilities)
        {
            if (ability.autocast)
                result.push_back(ability.id);
        }
        return result;
    }

    bool ShouldUseAbility(uint32 abilityId)
    {
        for (auto const& ability : context_.abilities)
        {
            if (ability.id != abilityId)
                continue;

            // Check cooldown
            if (ability.cooldownRemaining > 0)
                return false;

            // Must be in combat with target for most abilities
            if (ability.type != "defensive" && !context_.petHasTarget)
                return false;

            return true;
        }
        return false;
    }

    bool ShouldToggleGrowl(bool tankPetIsActive)
    {
        // Growl should only be on if pet is tank
        for (auto& ability : context_.abilities)
        {
            if (ability.name == "Growl")
                return ability.autocast != tankPetIsActive;
        }
        return false;
    }
};

TEST_F(PetAbilityTest, AutocastAbilitiesListed)
{
    auto autocast = GetAutocastAbilities();
    EXPECT_EQ(3u, autocast.size());  // Bite, Claw, Growl
}

TEST_F(PetAbilityTest, CanUseOffCooldown)
{
    EXPECT_TRUE(ShouldUseAbility(1));  // Bite ready
}

TEST_F(PetAbilityTest, CannotUseOnCooldown)
{
    context_.abilities[0].cooldownRemaining = 5000;
    EXPECT_FALSE(ShouldUseAbility(1));
}

TEST_F(PetAbilityTest, CannotUseDamageWithoutTarget)
{
    context_.petHasTarget = false;
    EXPECT_FALSE(ShouldUseAbility(1));
}

TEST_F(PetAbilityTest, CanUseDefensiveWithoutTarget)
{
    context_.petHasTarget = false;
    EXPECT_TRUE(ShouldUseAbility(4));  // Cower is defensive
}

TEST_F(PetAbilityTest, ToggleGrowlForTankPet)
{
    // Growl is currently on (autocast = true)
    // If pet is tank, it should stay on
    EXPECT_FALSE(ShouldToggleGrowl(true));

    // If pet is not tank, Growl should be toggled off
    EXPECT_TRUE(ShouldToggleGrowl(false));
}

/**
 * @brief Tests for pet positioning
 */
class PetPositioningTest : public ::testing::Test
{
protected:
    struct Position
    {
        float x, y;
    };

    struct PetPositionContext
    {
        Position ownerPos;
        Position targetPos;
        float ownerOrientation;
        bool isMeleePet;
        bool isRangedPet;
        float attackRange;
    };

    PetPositionContext context_;

    void SetUp() override
    {
        context_ = {
            {100.0f, 100.0f},
            {120.0f, 100.0f},
            0.0f,  // Facing east
            true,
            false,
            5.0f
        };
    }

    float Distance(Position const& a, Position const& b)
    {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    Position CalculateMeleePetPosition()
    {
        // Position behind target
        float angle = std::atan2(
            context_.targetPos.y - context_.ownerPos.y,
            context_.targetPos.x - context_.ownerPos.x
        );

        float behindAngle = angle + M_PI;  // Opposite side

        return {
            context_.targetPos.x + std::cos(behindAngle) * context_.attackRange,
            context_.targetPos.y + std::sin(behindAngle) * context_.attackRange
        };
    }

    Position CalculateRangedPetPosition()
    {
        // Position at max range, to the side
        float angle = std::atan2(
            context_.targetPos.y - context_.ownerPos.y,
            context_.targetPos.x - context_.ownerPos.x
        );

        float sideAngle = angle + M_PI / 2.0f;  // 90 degrees to side

        return {
            context_.targetPos.x + std::cos(sideAngle) * context_.attackRange,
            context_.targetPos.y + std::sin(sideAngle) * context_.attackRange
        };
    }

    Position CalculateFollowPosition()
    {
        // Behind owner at a set distance
        float behindAngle = context_.ownerOrientation + M_PI;
        return {
            context_.ownerPos.x + std::cos(behindAngle) * 3.0f,
            context_.ownerPos.y + std::sin(behindAngle) * 3.0f
        };
    }
};

TEST_F(PetPositioningTest, MeleePetBehindTarget)
{
    Position pos = CalculateMeleePetPosition();

    // Should be behind target (opposite side from owner)
    // Target at 120, owner at 100, so behind is around 125
    EXPECT_GT(pos.x, context_.targetPos.x);
    EXPECT_NEAR(context_.targetPos.y, pos.y, 0.1f);
}

TEST_F(PetPositioningTest, MeleePetAtMeleeRange)
{
    Position pos = CalculateMeleePetPosition();
    float dist = Distance(pos, context_.targetPos);
    EXPECT_NEAR(context_.attackRange, dist, 0.01f);
}

TEST_F(PetPositioningTest, RangedPetToSide)
{
    context_.isRangedPet = true;
    context_.isMeleePet = false;

    Position pos = CalculateRangedPetPosition();

    // Should be to the side (y different from target)
    EXPECT_NE(context_.targetPos.y, pos.y);
}

TEST_F(PetPositioningTest, FollowPositionBehindOwner)
{
    Position pos = CalculateFollowPosition();

    // Should be behind owner (opposite of orientation)
    // Owner facing east (0), so behind is west
    EXPECT_LT(pos.x, context_.ownerPos.x);
}

/**
 * @brief Tests for pet health management
 */
class PetHealthManagementTest : public ::testing::Test
{
protected:
    struct PetHealthContext
    {
        float petHealthPercent;
        float petMaxHealth;
        bool ownerCanHealPet;
        bool hasMendPet;
        uint32 mendPetCooldown;
        bool petInCombat;
    };

    PetHealthContext context_;

    void SetUp() override
    {
        context_ = {
            100.0f, 10000.0f, true, true, 0, true
        };
    }

    bool ShouldHealPet()
    {
        if (!context_.ownerCanHealPet)
            return false;
        if (context_.mendPetCooldown > 0)
            return false;
        return context_.petHealthPercent < 50.0f;
    }

    bool ShouldDismissPet()
    {
        // Dismiss if pet is about to die and we can't save it
        if (context_.petHealthPercent < 10.0f)
        {
            if (context_.mendPetCooldown > 5000)
                return true;
            if (!context_.ownerCanHealPet)
                return true;
        }
        return false;
    }

    bool ShouldRevivePet()
    {
        // Pet is dead (0% health)
        if (context_.petHealthPercent <= 0.0f)
            return !context_.petInCombat;  // Revive out of combat
        return false;
    }

    float CalculateHealPriority()
    {
        // Priority based on health deficit
        float deficit = 100.0f - context_.petHealthPercent;

        // Pet healing is lower priority than player healing
        return deficit * 0.5f;
    }
};

TEST_F(PetHealthManagementTest, HealBelowThreshold)
{
    context_.petHealthPercent = 40.0f;
    EXPECT_TRUE(ShouldHealPet());
}

TEST_F(PetHealthManagementTest, DontHealAboveThreshold)
{
    context_.petHealthPercent = 60.0f;
    EXPECT_FALSE(ShouldHealPet());
}

TEST_F(PetHealthManagementTest, DontHealOnCooldown)
{
    context_.petHealthPercent = 30.0f;
    context_.mendPetCooldown = 5000;
    EXPECT_FALSE(ShouldHealPet());
}

TEST_F(PetHealthManagementTest, DismissWhenDying)
{
    context_.petHealthPercent = 5.0f;
    context_.mendPetCooldown = 10000;
    EXPECT_TRUE(ShouldDismissPet());
}

TEST_F(PetHealthManagementTest, DontDismissWhenCanHeal)
{
    context_.petHealthPercent = 5.0f;
    context_.mendPetCooldown = 0;
    EXPECT_FALSE(ShouldDismissPet());
}

TEST_F(PetHealthManagementTest, ReviveOutOfCombat)
{
    context_.petHealthPercent = 0.0f;
    context_.petInCombat = false;
    EXPECT_TRUE(ShouldRevivePet());
}

TEST_F(PetHealthManagementTest, DontReviveInCombat)
{
    context_.petHealthPercent = 0.0f;
    context_.petInCombat = true;
    EXPECT_FALSE(ShouldRevivePet());
}

TEST_F(PetHealthManagementTest, HealPriorityLowerThanPlayer)
{
    context_.petHealthPercent = 40.0f;  // 60% deficit
    float priority = CalculateHealPriority();

    // Pet priority should be half of deficit (30)
    EXPECT_FLOAT_EQ(30.0f, priority);
}

