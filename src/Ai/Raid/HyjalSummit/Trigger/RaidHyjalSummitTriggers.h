/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDHYJALSUMMITTRIGGERS_H
#define _PLAYERBOT_RAIDHYJALSUMMITTRIGGERS_H

#include "Trigger.h"

// General

class HyjalSummitBotIsNotInCombatTrigger : public Trigger
{
public:
    HyjalSummitBotIsNotInCombatTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "hyjal summit bot is not in combat") {}
    bool IsActive() override;
};

// Rage Winterchill

class RageWinterchillPullingBossTrigger : public Trigger
{
public:
    RageWinterchillPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "rage winterchill pulling boss") {}
    bool IsActive() override;
};

class RageWinterchillBossEngagedByMainTankTrigger : public Trigger
{
public:
    RageWinterchillBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "rage winterchill boss engaged by main tank") {}
    bool IsActive() override;
};

class RageWinterchillBossCastsDeathAndDecayOnRangedTrigger : public Trigger
{
public:
    RageWinterchillBossCastsDeathAndDecayOnRangedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "rage winterchill boss casts death and decay on ranged") {}
    bool IsActive() override;
};

class RageWinterchillMeleeIsStandingInDeathAndDecayTrigger : public Trigger
{
public:
    RageWinterchillMeleeIsStandingInDeathAndDecayTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "rage winterchill melee is standing in death and decay") {}
    bool IsActive() override;
};

// Anetheron

class AnetheronPullingBossOrInfernalTrigger : public Trigger
{
public:
    AnetheronPullingBossOrInfernalTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "anetheron pulling boss or infernal") {}
    bool IsActive() override;
};

class AnetheronBossEngagedByMainTankTrigger : public Trigger
{
public:
    AnetheronBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "anetheron boss engaged by main tank") {}
    bool IsActive() override;
};

class AnetheronBossCastsCarrionSwarmTrigger : public Trigger
{
public:
    AnetheronBossCastsCarrionSwarmTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "anetheron boss casts carrion swarm") {}
    bool IsActive() override;
};

class AnetheronBotIsTargetedByInfernalTrigger : public Trigger
{
public:
    AnetheronBotIsTargetedByInfernalTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "anetheron bot is targeted by infernal") {}
    bool IsActive() override;
};

class AnetheronInfernalsNeedToBeKeptAwayFromRaidTrigger : public Trigger
{
public:
    AnetheronInfernalsNeedToBeKeptAwayFromRaidTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "anetheron infernals need to be kept away from raid") {}
    bool IsActive() override;
};

class AnetheronInfernalsContinueToSpawnTrigger : public Trigger
{
public:
    AnetheronInfernalsContinueToSpawnTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "anetheron infernals continue to spawn") {}
    bool IsActive() override;
};

// Kaz'rogal

class KazrogalPullingBossTrigger : public Trigger
{
public:
    KazrogalPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kaz'rogal pulling boss") {}
    bool IsActive() override;
};

class KazrogalBossEngagedByMainTankTrigger : public Trigger
{
public:
    KazrogalBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kaz'rogal boss engaged by main tank") {}
    bool IsActive() override;
};

class KazrogalBossEngagedByAssistTanksTrigger : public Trigger
{
public:
    KazrogalBossEngagedByAssistTanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kaz'rogal boss engaged by assist tanks") {}
    bool IsActive() override;
};

class KazrogalLowManaBotsNeedEscapePathTrigger : public Trigger
{
public:
    KazrogalLowManaBotsNeedEscapePathTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kaz'rogal low mana bots need escape path") {}
    bool IsActive() override;
};

class KazrogalBotIsLowOnManaTrigger : public Trigger
{
public:
    KazrogalBotIsLowOnManaTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kaz'rogal bot is low on mana") {}
    bool IsActive() override;
};

class KazrogalMarkDealsShadowDamageTrigger : public Trigger
{
public:
    KazrogalMarkDealsShadowDamageTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kaz'rogal mark deals shadow damage") {}
    bool IsActive() override;
};

// Azgalor

class AzgalorPullingBossTrigger : public Trigger
{
public:
    AzgalorPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "azgalor pulling boss") {}
    bool IsActive() override;
};

class AzgalorBossEngagedByMainTankTrigger : public Trigger
{
public:
    AzgalorBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "azgalor boss engaged by main tank") {}
    bool IsActive() override;
};

class AzgalorMainTankIsPositioningBossTrigger : public Trigger
{
public:
    AzgalorMainTankIsPositioningBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "azgalor main tank is positioning boss") {}
    bool IsActive() override;
};

class AzgalorBossEngagedByRangedTrigger : public Trigger
{
public:
    AzgalorBossEngagedByRangedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "azgalor boss engaged by ranged") {}
    bool IsActive() override;
};

class AzgalorBossCastsRainOfFireOnMeleeTrigger : public Trigger
{
public:
    AzgalorBossCastsRainOfFireOnMeleeTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "azgalor boss casts rain of fire on melee") {}
    bool IsActive() override;
};

class AzgalorBotIsDoomedTrigger : public Trigger
{
public:
    AzgalorBotIsDoomedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "azgalor bot is doomed") {}
    bool IsActive() override;
};

class AzgalorDoomguardsMustBeControlledTrigger : public Trigger
{
public:
    AzgalorDoomguardsMustBeControlledTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "azgalor doomguards must be controlled") {}
    bool IsActive() override;
};

class AzgalorDoomguardsMustDieTrigger : public Trigger
{
public:
    AzgalorDoomguardsMustDieTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "azgalor doomguards must die") {}
    bool IsActive() override;
};

// Archimonde

class ArchimondePullingBossTrigger : public Trigger
{
public:
    ArchimondePullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "archimonde pulling boss") {}
    bool IsActive() override;
};

class ArchimondeBossEngagedByMainTankTrigger : public Trigger
{
public:
    ArchimondeBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "archimonde boss engaged by main tank") {}
    bool IsActive() override;
};

class ArchimondeBossCastsFearTrigger : public Trigger
{
public:
    ArchimondeBossCastsFearTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "archimonde boss casts fear") {}
    bool IsActive() override;
};

class ArchimondeBossCastsAirBurstTrigger : public Trigger
{
public:
    ArchimondeBossCastsAirBurstTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "archimonde boss casts air burst") {}
    bool IsActive() override;
};

class ArchimondeBossSummonedDoomfireTrigger : public Trigger
{
public:
    ArchimondeBossSummonedDoomfireTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "archimonde boss summoned doomfire") {}
    bool IsActive() override;
};

class ArchimondeBotStoodInDoomfireTrigger : public Trigger
{
public:
    ArchimondeBotStoodInDoomfireTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "archimonde bot stood in doomfire") {}
    bool IsActive() override;
};

#endif
