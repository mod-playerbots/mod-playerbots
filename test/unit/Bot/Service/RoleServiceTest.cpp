/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "mocks/MockBotServices.h"

using ::testing::Return;
using ::testing::_;

/**
 * @brief Tests for the MockRoleService
 *
 * These tests demonstrate how to use the mock role service
 * for testing bot components that depend on role detection.
 */
class RoleServiceTest : public ::testing::Test
{
protected:
    MockRoleService mockRoleService;
};

TEST_F(RoleServiceTest, CanMockTankRole)
{
    // Set up mock to return true for IsTank
    EXPECT_CALL(mockRoleService, IsTank(_, false)).WillOnce(Return(true));
    EXPECT_CALL(mockRoleService, IsHeal(_, false)).WillOnce(Return(false));
    EXPECT_CALL(mockRoleService, IsDps(_, false)).WillOnce(Return(false));

    EXPECT_TRUE(mockRoleService.IsTank(nullptr, false));
    EXPECT_FALSE(mockRoleService.IsHeal(nullptr, false));
    EXPECT_FALSE(mockRoleService.IsDps(nullptr, false));
}

TEST_F(RoleServiceTest, CanMockHealerRole)
{
    EXPECT_CALL(mockRoleService, IsTank(_, _)).WillRepeatedly(Return(false));
    EXPECT_CALL(mockRoleService, IsHeal(_, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(mockRoleService, IsDps(_, _)).WillRepeatedly(Return(false));

    EXPECT_FALSE(mockRoleService.IsTank(nullptr));
    EXPECT_TRUE(mockRoleService.IsHeal(nullptr));
    EXPECT_FALSE(mockRoleService.IsDps(nullptr));
}

TEST_F(RoleServiceTest, CanMockDpsRole)
{
    EXPECT_CALL(mockRoleService, IsTank(_, _)).WillRepeatedly(Return(false));
    EXPECT_CALL(mockRoleService, IsHeal(_, _)).WillRepeatedly(Return(false));
    EXPECT_CALL(mockRoleService, IsDps(_, _)).WillRepeatedly(Return(true));

    EXPECT_TRUE(mockRoleService.IsDps(nullptr));
}

TEST_F(RoleServiceTest, CanMockCombatStyle)
{
    // Ranged caster
    EXPECT_CALL(mockRoleService, IsRanged(_, _)).WillOnce(Return(true));
    EXPECT_CALL(mockRoleService, IsMelee(_, _)).WillOnce(Return(false));
    EXPECT_CALL(mockRoleService, IsCaster(_, _)).WillOnce(Return(true));

    EXPECT_TRUE(mockRoleService.IsRanged(nullptr));
    EXPECT_FALSE(mockRoleService.IsMelee(nullptr));
    EXPECT_TRUE(mockRoleService.IsCaster(nullptr));
}

TEST_F(RoleServiceTest, CanMockTankHierarchy)
{
    // Main tank
    EXPECT_CALL(mockRoleService, IsMainTank(_)).WillOnce(Return(true));
    EXPECT_CALL(mockRoleService, IsAssistTank(_)).WillOnce(Return(false));
    EXPECT_CALL(mockRoleService, IsBotMainTank(_)).WillOnce(Return(true));

    EXPECT_TRUE(mockRoleService.IsMainTank(nullptr));
    EXPECT_FALSE(mockRoleService.IsAssistTank(nullptr));
    EXPECT_TRUE(mockRoleService.IsBotMainTank(nullptr));
}

TEST_F(RoleServiceTest, CanMockGroupQueries)
{
    EXPECT_CALL(mockRoleService, GetGroupTankNum(_)).WillOnce(Return(2));
    EXPECT_CALL(mockRoleService, GetAssistTankIndex(_)).WillOnce(Return(1));
    EXPECT_CALL(mockRoleService, GetGroupSlotIndex(_)).WillOnce(Return(3));

    EXPECT_EQ(2u, mockRoleService.GetGroupTankNum(nullptr));
    EXPECT_EQ(1, mockRoleService.GetAssistTankIndex(nullptr));
    EXPECT_EQ(3, mockRoleService.GetGroupSlotIndex(nullptr));
}

/**
 * @brief Tests for role-based behavior patterns
 */
class RoleBehaviorPatternTest : public ::testing::Test
{
protected:
    MockRoleService mockRoleService;

    /**
     * @brief Determine if a player should use melee attacks
     */
    bool ShouldUseMeleeAttacks(MockRoleService& roleService, void* player)
    {
        return roleService.IsMelee(static_cast<Player*>(player));
    }

    /**
     * @brief Determine if a player should stay at range
     */
    bool ShouldStayAtRange(MockRoleService& roleService, void* player)
    {
        return roleService.IsRanged(static_cast<Player*>(player));
    }

    /**
     * @brief Determine if a player should heal others
     */
    bool ShouldHealOthers(MockRoleService& roleService, void* player)
    {
        return roleService.IsHeal(static_cast<Player*>(player));
    }

    /**
     * @brief Determine if a player should taunt
     */
    bool ShouldTaunt(MockRoleService& roleService, void* player)
    {
        auto* p = static_cast<Player*>(player);
        return roleService.IsTank(p) && roleService.IsMainTank(p);
    }
};

TEST_F(RoleBehaviorPatternTest, MeleeWarriorBehavior)
{
    EXPECT_CALL(mockRoleService, IsMelee(_, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(mockRoleService, IsRanged(_, _)).WillRepeatedly(Return(false));
    EXPECT_CALL(mockRoleService, IsTank(_, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(mockRoleService, IsMainTank(_)).WillRepeatedly(Return(true));

    EXPECT_TRUE(ShouldUseMeleeAttacks(mockRoleService, nullptr));
    EXPECT_FALSE(ShouldStayAtRange(mockRoleService, nullptr));
    EXPECT_TRUE(ShouldTaunt(mockRoleService, nullptr));
}

TEST_F(RoleBehaviorPatternTest, RangedHealerBehavior)
{
    EXPECT_CALL(mockRoleService, IsMelee(_, _)).WillRepeatedly(Return(false));
    EXPECT_CALL(mockRoleService, IsRanged(_, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(mockRoleService, IsHeal(_, _)).WillRepeatedly(Return(true));

    EXPECT_FALSE(ShouldUseMeleeAttacks(mockRoleService, nullptr));
    EXPECT_TRUE(ShouldStayAtRange(mockRoleService, nullptr));
    EXPECT_TRUE(ShouldHealOthers(mockRoleService, nullptr));
}

TEST_F(RoleBehaviorPatternTest, OffTankBehavior)
{
    EXPECT_CALL(mockRoleService, IsTank(_, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(mockRoleService, IsMainTank(_)).WillRepeatedly(Return(false));

    // Off-tank should not taunt (only main tank taunts)
    EXPECT_FALSE(ShouldTaunt(mockRoleService, nullptr));
}
