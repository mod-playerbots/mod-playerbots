/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CONFIG_PROVIDER_H
#define _PLAYERBOT_CONFIG_PROVIDER_H

#include "Bot/Interface/IConfigProvider.h"
#include "PlayerbotAIConfig.h"

/**
 * @brief Implementation of IConfigProvider that wraps PlayerbotAIConfig
 *
 * This class provides a testable interface to the PlayerbotAIConfig singleton.
 */
class ConfigProvider : public IConfigProvider
{
public:
    ConfigProvider() = default;
    ~ConfigProvider() override = default;

    // Distance settings
    float GetSightDistance() const override { return sPlayerbotAIConfig->sightDistance; }
    float GetSpellDistance() const override { return sPlayerbotAIConfig->spellDistance; }
    float GetReactDistance() const override { return sPlayerbotAIConfig->reactDistance; }
    float GetGrindDistance() const override { return sPlayerbotAIConfig->grindDistance; }
    float GetLootDistance() const override { return sPlayerbotAIConfig->lootDistance; }
    float GetShootDistance() const override { return sPlayerbotAIConfig->shootDistance; }
    float GetFleeDistance() const override { return sPlayerbotAIConfig->fleeDistance; }
    float GetTooCloseDistance() const override { return sPlayerbotAIConfig->tooCloseDistance; }
    float GetMeleeDistance() const override { return sPlayerbotAIConfig->meleeDistance; }
    float GetFollowDistance() const override { return sPlayerbotAIConfig->followDistance; }
    float GetWhisperDistance() const override { return sPlayerbotAIConfig->whisperDistance; }
    float GetContactDistance() const override { return sPlayerbotAIConfig->contactDistance; }
    float GetAoeRadius() const override { return sPlayerbotAIConfig->aoeRadius; }
    float GetRpgDistance() const override { return sPlayerbotAIConfig->rpgDistance; }
    float GetTargetPosRecalcDistance() const override { return sPlayerbotAIConfig->targetPosRecalcDistance; }
    float GetFarDistance() const override { return sPlayerbotAIConfig->farDistance; }
    float GetHealDistance() const override { return sPlayerbotAIConfig->healDistance; }
    float GetAggroDistance() const override { return sPlayerbotAIConfig->aggroDistance; }

    // Timing settings
    uint32 GetGlobalCooldown() const override { return sPlayerbotAIConfig->globalCoolDown; }
    uint32 GetReactDelay() const override { return sPlayerbotAIConfig->reactDelay; }
    uint32 GetMaxWaitForMove() const override { return sPlayerbotAIConfig->maxWaitForMove; }
    uint32 GetExpireActionTime() const override { return sPlayerbotAIConfig->expireActionTime; }
    uint32 GetDispelAuraDuration() const override { return sPlayerbotAIConfig->dispelAuraDuration; }
    uint32 GetPassiveDelay() const override { return sPlayerbotAIConfig->passiveDelay; }
    uint32 GetRepeatDelay() const override { return sPlayerbotAIConfig->repeatDelay; }
    uint32 GetErrorDelay() const override { return sPlayerbotAIConfig->errorDelay; }
    uint32 GetRpgDelay() const override { return sPlayerbotAIConfig->rpgDelay; }
    uint32 GetSitDelay() const override { return sPlayerbotAIConfig->sitDelay; }
    uint32 GetReturnDelay() const override { return sPlayerbotAIConfig->returnDelay; }
    uint32 GetLootDelay() const override { return sPlayerbotAIConfig->lootDelay; }

    // Health/Mana thresholds
    uint32 GetCriticalHealth() const override { return sPlayerbotAIConfig->criticalHealth; }
    uint32 GetLowHealth() const override { return sPlayerbotAIConfig->lowHealth; }
    uint32 GetMediumHealth() const override { return sPlayerbotAIConfig->mediumHealth; }
    uint32 GetAlmostFullHealth() const override { return sPlayerbotAIConfig->almostFullHealth; }
    uint32 GetLowMana() const override { return sPlayerbotAIConfig->lowMana; }
    uint32 GetMediumMana() const override { return sPlayerbotAIConfig->mediumMana; }
    uint32 GetHighMana() const override { return sPlayerbotAIConfig->highMana; }

    // Feature flags
    bool IsEnabled() const override { return sPlayerbotAIConfig->enabled; }
    bool IsDynamicReactDelay() const override { return sPlayerbotAIConfig->dynamicReactDelay; }
    bool IsAutoSaveMana() const override { return sPlayerbotAIConfig->autoSaveMana; }
    uint32 GetSaveManaThreshold() const override { return sPlayerbotAIConfig->saveManaThreshold; }
    bool IsAutoAvoidAoe() const override { return sPlayerbotAIConfig->autoAvoidAoe; }
    float GetMaxAoeAvoidRadius() const override { return sPlayerbotAIConfig->maxAoeAvoidRadius; }
    bool IsTellWhenAvoidAoe() const override { return sPlayerbotAIConfig->tellWhenAvoidAoe; }

    // Bot behavior settings
    bool IsFleeingEnabled() const override { return sPlayerbotAIConfig->fleeingEnabled; }
    bool IsRandomBotAutologin() const override { return sPlayerbotAIConfig->randomBotAutologin; }
    bool IsBotAutologin() const override { return sPlayerbotAIConfig->botAutologin; }

    // Logging
    bool HasLog(std::string const& fileName) const override { return sPlayerbotAIConfig->hasLog(fileName); }
};

#endif
