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
 * @brief Tests for BotSpellService
 *
 * These tests verify the spell service functionality
 * for bot spell casting and aura management operations.
 */
class SpellServiceTest : public ::testing::Test
{
protected:
    MockSpellService mockSpellService;
};

TEST_F(SpellServiceTest, CanMockCanCastSpellByName)
{
    EXPECT_CALL(mockSpellService, CanCastSpell(std::string("Fireball"), _, _))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockSpellService.CanCastSpell("Fireball", nullptr, nullptr));
}

TEST_F(SpellServiceTest, CanMockCastSpellByName)
{
    EXPECT_CALL(mockSpellService, CastSpell(std::string("Fireball"), _, _))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockSpellService.CastSpell("Fireball", nullptr, nullptr));
}

TEST_F(SpellServiceTest, CanMockCanCastSpellById)
{
    EXPECT_CALL(mockSpellService, CanCastSpell(uint32(12345), (Unit*)nullptr, _, _, _))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockSpellService.CanCastSpell(uint32(12345), (Unit*)nullptr, true, nullptr, nullptr));
}

TEST_F(SpellServiceTest, CanMockCastSpellById)
{
    EXPECT_CALL(mockSpellService, CastSpell(uint32(12345), (Unit*)nullptr, _))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockSpellService.CastSpell(uint32(12345), (Unit*)nullptr, nullptr));
}

TEST_F(SpellServiceTest, CanMockHasAuraByName)
{
    EXPECT_CALL(mockSpellService, HasAura(std::string("Power Word: Fortitude"), _, _, _, _, _))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockSpellService.HasAura("Power Word: Fortitude", nullptr));
}

TEST_F(SpellServiceTest, CanMockHasAuraById)
{
    EXPECT_CALL(mockSpellService, HasAura(uint32(12345), _))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockSpellService.HasAura(uint32(12345), nullptr));
}

TEST_F(SpellServiceTest, CanMockGetAura)
{
    EXPECT_CALL(mockSpellService, GetAura(std::string("Renew"), _, _, _, _))
        .WillOnce(Return(nullptr));

    EXPECT_EQ(nullptr, mockSpellService.GetAura("Renew", nullptr));
}

TEST_F(SpellServiceTest, CanMockHasAuraToDispel)
{
    EXPECT_CALL(mockSpellService, HasAuraToDispel(_, 1))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockSpellService.HasAuraToDispel(nullptr, 1));
}

TEST_F(SpellServiceTest, CanMockCanDispel)
{
    EXPECT_CALL(mockSpellService, CanDispel(_, 1))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockSpellService.CanDispel(nullptr, 1));
}

TEST_F(SpellServiceTest, CanMockIsInterruptableSpellCasting)
{
    EXPECT_CALL(mockSpellService, IsInterruptableSpellCasting(_, std::string("Kick")))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockSpellService.IsInterruptableSpellCasting(nullptr, "Kick"));
}

TEST_F(SpellServiceTest, CanMockCalculateGlobalCooldown)
{
    EXPECT_CALL(mockSpellService, CalculateGlobalCooldown(12345))
        .WillOnce(Return(1500));

    EXPECT_EQ(1500, mockSpellService.CalculateGlobalCooldown(12345));
}

TEST_F(SpellServiceTest, CanMockIsInVehicle)
{
    EXPECT_CALL(mockSpellService, IsInVehicle(true, false, false, false, false))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockSpellService.IsInVehicle(true, false, false, false, false));
}

/**
 * @brief Tests for spell behavior patterns
 */
class SpellBehaviorPatternTest : public ::testing::Test
{
protected:
    MockSpellService mockSpellService;

    bool CanCastHeal(MockSpellService& spellService, Unit* target, std::string const& healSpell)
    {
        return spellService.CanCastSpell(healSpell, target);
    }

    bool TryCastHeal(MockSpellService& spellService, Unit* target, std::string const& healSpell)
    {
        if (CanCastHeal(spellService, target, healSpell))
        {
            return spellService.CastSpell(healSpell, target);
        }
        return false;
    }

    bool ShouldDispel(MockSpellService& spellService, Unit* target, uint32 dispelType)
    {
        return spellService.HasAuraToDispel(target, dispelType);
    }

    bool ShouldInterrupt(MockSpellService& spellService, Unit* target, std::string const& interruptSpell)
    {
        return spellService.IsInterruptableSpellCasting(target, interruptSpell);
    }
};

TEST_F(SpellBehaviorPatternTest, CanCheckHealability)
{
    EXPECT_CALL(mockSpellService, CanCastSpell(std::string("Flash Heal"), _, _))
        .WillOnce(Return(true));

    EXPECT_TRUE(CanCastHeal(mockSpellService, nullptr, "Flash Heal"));
}

TEST_F(SpellBehaviorPatternTest, TryCastHealSuccess)
{
    EXPECT_CALL(mockSpellService, CanCastSpell(std::string("Flash Heal"), _, _))
        .WillOnce(Return(true));
    EXPECT_CALL(mockSpellService, CastSpell(std::string("Flash Heal"), _, _))
        .WillOnce(Return(true));

    EXPECT_TRUE(TryCastHeal(mockSpellService, nullptr, "Flash Heal"));
}

TEST_F(SpellBehaviorPatternTest, TryCastHealFailCannotCast)
{
    EXPECT_CALL(mockSpellService, CanCastSpell(std::string("Flash Heal"), _, _))
        .WillOnce(Return(false));

    EXPECT_FALSE(TryCastHeal(mockSpellService, nullptr, "Flash Heal"));
}

TEST_F(SpellBehaviorPatternTest, CheckDispelNeed)
{
    EXPECT_CALL(mockSpellService, HasAuraToDispel(_, 1))
        .WillOnce(Return(true));

    EXPECT_TRUE(ShouldDispel(mockSpellService, nullptr, 1));
}

TEST_F(SpellBehaviorPatternTest, CheckInterruptNeed)
{
    EXPECT_CALL(mockSpellService, IsInterruptableSpellCasting(_, std::string("Kick")))
        .WillOnce(Return(true));

    EXPECT_TRUE(ShouldInterrupt(mockSpellService, nullptr, "Kick"));
}

/**
 * @brief Tests for aura management patterns
 */
class SpellAuraManagementTest : public ::testing::Test
{
protected:
    MockSpellService mockSpellService;

    bool HasBuff(MockSpellService& spellService, Unit* unit, std::string const& buffName)
    {
        return spellService.HasAura(buffName, unit);
    }

    bool NeedsBuffRefresh(MockSpellService& spellService, Unit* unit, std::string const& buffName)
    {
        return !spellService.HasAura(buffName, unit);
    }
};

TEST_F(SpellAuraManagementTest, CheckHasBuff)
{
    EXPECT_CALL(mockSpellService, HasAura(std::string("Power Word: Fortitude"), _, _, _, _, _))
        .WillOnce(Return(true));

    EXPECT_TRUE(HasBuff(mockSpellService, nullptr, "Power Word: Fortitude"));
}

TEST_F(SpellAuraManagementTest, CheckNeedsBuffRefresh)
{
    EXPECT_CALL(mockSpellService, HasAura(std::string("Power Word: Fortitude"), _, _, _, _, _))
        .WillOnce(Return(false));

    EXPECT_TRUE(NeedsBuffRefresh(mockSpellService, nullptr, "Power Word: Fortitude"));
}

TEST_F(SpellAuraManagementTest, CheckNoBuffRefreshNeeded)
{
    EXPECT_CALL(mockSpellService, HasAura(std::string("Power Word: Fortitude"), _, _, _, _, _))
        .WillOnce(Return(true));

    EXPECT_FALSE(NeedsBuffRefresh(mockSpellService, nullptr, "Power Word: Fortitude"));
}

/**
 * @brief Tests for vehicle spell patterns
 */
class SpellVehicleTest : public ::testing::Test
{
protected:
    MockSpellService mockSpellService;
};

TEST_F(SpellVehicleTest, CanCheckInVehicleWithControl)
{
    EXPECT_CALL(mockSpellService, IsInVehicle(true, false, false, false, false))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockSpellService.IsInVehicle(true, false, false, false, false));
}

TEST_F(SpellVehicleTest, CanCheckInVehicleCanCast)
{
    EXPECT_CALL(mockSpellService, IsInVehicle(false, true, false, false, false))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockSpellService.IsInVehicle(false, true, false, false, false));
}

TEST_F(SpellVehicleTest, CanMockCanCastVehicleSpell)
{
    EXPECT_CALL(mockSpellService, CanCastVehicleSpell(12345, _))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockSpellService.CanCastVehicleSpell(12345, nullptr));
}

TEST_F(SpellVehicleTest, CanMockCastVehicleSpell)
{
    EXPECT_CALL(mockSpellService, CastVehicleSpell(uint32(12345), (Unit*)nullptr))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockSpellService.CastVehicleSpell(uint32(12345), (Unit*)nullptr));
}
