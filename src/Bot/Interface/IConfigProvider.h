/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ICONFIG_PROVIDER_H
#define _PLAYERBOT_ICONFIG_PROVIDER_H

#include "Common.h"

/**
 * @brief Interface for bot configuration access
 *
 * This interface abstracts access to bot configuration settings,
 * allowing for dependency injection and easier testing.
 */
class IConfigProvider
{
public:
    virtual ~IConfigProvider() = default;

    // Distance settings
    virtual float GetSightDistance() const = 0;
    virtual float GetSpellDistance() const = 0;
    virtual float GetReactDistance() const = 0;
    virtual float GetGrindDistance() const = 0;
    virtual float GetLootDistance() const = 0;
    virtual float GetShootDistance() const = 0;
    virtual float GetFleeDistance() const = 0;
    virtual float GetTooCloseDistance() const = 0;
    virtual float GetMeleeDistance() const = 0;
    virtual float GetFollowDistance() const = 0;
    virtual float GetWhisperDistance() const = 0;
    virtual float GetContactDistance() const = 0;
    virtual float GetAoeRadius() const = 0;
    virtual float GetRpgDistance() const = 0;
    virtual float GetTargetPosRecalcDistance() const = 0;
    virtual float GetFarDistance() const = 0;
    virtual float GetHealDistance() const = 0;
    virtual float GetAggroDistance() const = 0;

    // Timing settings
    virtual uint32 GetGlobalCooldown() const = 0;
    virtual uint32 GetReactDelay() const = 0;
    virtual uint32 GetMaxWaitForMove() const = 0;
    virtual uint32 GetExpireActionTime() const = 0;
    virtual uint32 GetDispelAuraDuration() const = 0;
    virtual uint32 GetPassiveDelay() const = 0;
    virtual uint32 GetRepeatDelay() const = 0;
    virtual uint32 GetErrorDelay() const = 0;
    virtual uint32 GetRpgDelay() const = 0;
    virtual uint32 GetSitDelay() const = 0;
    virtual uint32 GetReturnDelay() const = 0;
    virtual uint32 GetLootDelay() const = 0;

    // Health/Mana thresholds
    virtual uint32 GetCriticalHealth() const = 0;
    virtual uint32 GetLowHealth() const = 0;
    virtual uint32 GetMediumHealth() const = 0;
    virtual uint32 GetAlmostFullHealth() const = 0;
    virtual uint32 GetLowMana() const = 0;
    virtual uint32 GetMediumMana() const = 0;
    virtual uint32 GetHighMana() const = 0;

    // Feature flags
    virtual bool IsEnabled() const = 0;
    virtual bool IsDynamicReactDelay() const = 0;
    virtual bool IsAutoSaveMana() const = 0;
    virtual uint32 GetSaveManaThreshold() const = 0;
    virtual bool IsAutoAvoidAoe() const = 0;
    virtual float GetMaxAoeAvoidRadius() const = 0;
    virtual bool IsTellWhenAvoidAoe() const = 0;

    // Bot behavior settings
    virtual bool IsFleeingEnabled() const = 0;
    virtual bool IsRandomBotAutologin() const = 0;
    virtual bool IsBotAutologin() const = 0;

    // Logging
    virtual bool HasLog(std::string const& fileName) const = 0;
};

#endif
