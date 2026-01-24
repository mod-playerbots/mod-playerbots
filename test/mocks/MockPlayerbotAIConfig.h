/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MOCK_PLAYERBOT_AI_CONFIG_H
#define _PLAYERBOT_MOCK_PLAYERBOT_AI_CONFIG_H

#include <gmock/gmock.h>
#include "Bot/Interface/IConfigProvider.h"

/**
 * @brief Mock implementation of IConfigProvider for testing
 *
 * This mock allows tests to inject specific configuration values
 * without depending on the PlayerbotAIConfig singleton.
 */
class MockConfigProvider : public IConfigProvider
{
public:
    // Distance settings
    MOCK_METHOD(float, GetSightDistance, (), (const, override));
    MOCK_METHOD(float, GetSpellDistance, (), (const, override));
    MOCK_METHOD(float, GetReactDistance, (), (const, override));
    MOCK_METHOD(float, GetGrindDistance, (), (const, override));
    MOCK_METHOD(float, GetLootDistance, (), (const, override));
    MOCK_METHOD(float, GetShootDistance, (), (const, override));
    MOCK_METHOD(float, GetFleeDistance, (), (const, override));
    MOCK_METHOD(float, GetTooCloseDistance, (), (const, override));
    MOCK_METHOD(float, GetMeleeDistance, (), (const, override));
    MOCK_METHOD(float, GetFollowDistance, (), (const, override));
    MOCK_METHOD(float, GetWhisperDistance, (), (const, override));
    MOCK_METHOD(float, GetContactDistance, (), (const, override));
    MOCK_METHOD(float, GetAoeRadius, (), (const, override));
    MOCK_METHOD(float, GetRpgDistance, (), (const, override));
    MOCK_METHOD(float, GetTargetPosRecalcDistance, (), (const, override));
    MOCK_METHOD(float, GetFarDistance, (), (const, override));
    MOCK_METHOD(float, GetHealDistance, (), (const, override));
    MOCK_METHOD(float, GetAggroDistance, (), (const, override));

    // Timing settings
    MOCK_METHOD(uint32, GetGlobalCooldown, (), (const, override));
    MOCK_METHOD(uint32, GetReactDelay, (), (const, override));
    MOCK_METHOD(uint32, GetMaxWaitForMove, (), (const, override));
    MOCK_METHOD(uint32, GetExpireActionTime, (), (const, override));
    MOCK_METHOD(uint32, GetDispelAuraDuration, (), (const, override));
    MOCK_METHOD(uint32, GetPassiveDelay, (), (const, override));
    MOCK_METHOD(uint32, GetRepeatDelay, (), (const, override));
    MOCK_METHOD(uint32, GetErrorDelay, (), (const, override));
    MOCK_METHOD(uint32, GetRpgDelay, (), (const, override));
    MOCK_METHOD(uint32, GetSitDelay, (), (const, override));
    MOCK_METHOD(uint32, GetReturnDelay, (), (const, override));
    MOCK_METHOD(uint32, GetLootDelay, (), (const, override));

    // Health/Mana thresholds
    MOCK_METHOD(uint32, GetCriticalHealth, (), (const, override));
    MOCK_METHOD(uint32, GetLowHealth, (), (const, override));
    MOCK_METHOD(uint32, GetMediumHealth, (), (const, override));
    MOCK_METHOD(uint32, GetAlmostFullHealth, (), (const, override));
    MOCK_METHOD(uint32, GetLowMana, (), (const, override));
    MOCK_METHOD(uint32, GetMediumMana, (), (const, override));
    MOCK_METHOD(uint32, GetHighMana, (), (const, override));

    // Feature flags
    MOCK_METHOD(bool, IsEnabled, (), (const, override));
    MOCK_METHOD(bool, IsDynamicReactDelay, (), (const, override));
    MOCK_METHOD(bool, IsAutoSaveMana, (), (const, override));
    MOCK_METHOD(uint32, GetSaveManaThreshold, (), (const, override));
    MOCK_METHOD(bool, IsAutoAvoidAoe, (), (const, override));
    MOCK_METHOD(float, GetMaxAoeAvoidRadius, (), (const, override));
    MOCK_METHOD(bool, IsTellWhenAvoidAoe, (), (const, override));

    // Bot behavior settings
    MOCK_METHOD(bool, IsFleeingEnabled, (), (const, override));
    MOCK_METHOD(bool, IsRandomBotAutologin, (), (const, override));
    MOCK_METHOD(bool, IsBotAutologin, (), (const, override));

    // Logging
    MOCK_METHOD(bool, HasLog, (std::string const& fileName), (const, override));

    /**
     * @brief Set up default return values for common configuration
     *
     * Call this in test setup to get reasonable defaults without
     * having to mock every method.
     */
    void SetupDefaults()
    {
        using ::testing::Return;

        // Distance defaults
        ON_CALL(*this, GetSightDistance()).WillByDefault(Return(75.0f));
        ON_CALL(*this, GetSpellDistance()).WillByDefault(Return(26.0f));
        ON_CALL(*this, GetReactDistance()).WillByDefault(Return(150.0f));
        ON_CALL(*this, GetGrindDistance()).WillByDefault(Return(100.0f));
        ON_CALL(*this, GetLootDistance()).WillByDefault(Return(15.0f));
        ON_CALL(*this, GetShootDistance()).WillByDefault(Return(26.0f));
        ON_CALL(*this, GetFleeDistance()).WillByDefault(Return(20.0f));
        ON_CALL(*this, GetTooCloseDistance()).WillByDefault(Return(5.0f));
        ON_CALL(*this, GetMeleeDistance()).WillByDefault(Return(1.5f));
        ON_CALL(*this, GetFollowDistance()).WillByDefault(Return(1.5f));
        ON_CALL(*this, GetWhisperDistance()).WillByDefault(Return(6000.0f));
        ON_CALL(*this, GetContactDistance()).WillByDefault(Return(0.5f));
        ON_CALL(*this, GetAoeRadius()).WillByDefault(Return(5.0f));
        ON_CALL(*this, GetRpgDistance()).WillByDefault(Return(200.0f));
        ON_CALL(*this, GetTargetPosRecalcDistance()).WillByDefault(Return(0.1f));
        ON_CALL(*this, GetFarDistance()).WillByDefault(Return(20.0f));
        ON_CALL(*this, GetHealDistance()).WillByDefault(Return(38.0f));
        ON_CALL(*this, GetAggroDistance()).WillByDefault(Return(22.0f));

        // Timing defaults
        ON_CALL(*this, GetGlobalCooldown()).WillByDefault(Return(1500));
        ON_CALL(*this, GetReactDelay()).WillByDefault(Return(100));
        ON_CALL(*this, GetMaxWaitForMove()).WillByDefault(Return(5000));
        ON_CALL(*this, GetExpireActionTime()).WillByDefault(Return(5000));
        ON_CALL(*this, GetDispelAuraDuration()).WillByDefault(Return(2000));
        ON_CALL(*this, GetPassiveDelay()).WillByDefault(Return(4000));
        ON_CALL(*this, GetRepeatDelay()).WillByDefault(Return(5000));
        ON_CALL(*this, GetErrorDelay()).WillByDefault(Return(100));
        ON_CALL(*this, GetRpgDelay()).WillByDefault(Return(10000));
        ON_CALL(*this, GetSitDelay()).WillByDefault(Return(30000));
        ON_CALL(*this, GetReturnDelay()).WillByDefault(Return(3000));
        ON_CALL(*this, GetLootDelay()).WillByDefault(Return(1000));

        // Health/Mana thresholds
        ON_CALL(*this, GetCriticalHealth()).WillByDefault(Return(20));
        ON_CALL(*this, GetLowHealth()).WillByDefault(Return(45));
        ON_CALL(*this, GetMediumHealth()).WillByDefault(Return(65));
        ON_CALL(*this, GetAlmostFullHealth()).WillByDefault(Return(85));
        ON_CALL(*this, GetLowMana()).WillByDefault(Return(15));
        ON_CALL(*this, GetMediumMana()).WillByDefault(Return(40));
        ON_CALL(*this, GetHighMana()).WillByDefault(Return(80));

        // Feature flags
        ON_CALL(*this, IsEnabled()).WillByDefault(Return(true));
        ON_CALL(*this, IsDynamicReactDelay()).WillByDefault(Return(false));
        ON_CALL(*this, IsAutoSaveMana()).WillByDefault(Return(false));
        ON_CALL(*this, GetSaveManaThreshold()).WillByDefault(Return(50));
        ON_CALL(*this, IsAutoAvoidAoe()).WillByDefault(Return(true));
        ON_CALL(*this, GetMaxAoeAvoidRadius()).WillByDefault(Return(10.0f));
        ON_CALL(*this, IsTellWhenAvoidAoe()).WillByDefault(Return(true));

        // Bot behavior
        ON_CALL(*this, IsFleeingEnabled()).WillByDefault(Return(true));
        ON_CALL(*this, IsRandomBotAutologin()).WillByDefault(Return(false));
        ON_CALL(*this, IsBotAutologin()).WillByDefault(Return(false));

        // Logging
        ON_CALL(*this, HasLog(::testing::_)).WillByDefault(Return(false));
    }
};

#endif
