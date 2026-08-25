/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TKTRIGGERS_H
#define PLAYERBOTS_TKTRIGGERS_H

#include "Trigger.h"

// Trigger names describe the mechanic the paired action answers, not the condition tested here.
// Most boss abilities are periodic and cannot be seen coming, so a trigger is usually a coarse
// gate -- this boss is up, and this bot is the role that cares -- with any precise check living
// in the action. Read alongside TKStrategy.cpp, the pairs document the encounter.

// General

class TempestKeepNoEncounterInProgressTrigger : public Trigger
{
public:
    TempestKeepNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "tempest keep no encounter in progress") {}
    bool IsActive() override;
};

class TempestKeepBotIsStuckFallingTrigger : public Trigger
{
public:
    TempestKeepBotIsStuckFallingTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "tempest keep bot is stuck falling") {}
    bool IsActive() override;
};

// Trash

class CrimsonHandCenturionCastsArcaneFlurryTrigger : public Trigger
{
public:
    CrimsonHandCenturionCastsArcaneFlurryTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "crimson hand centurion casts arcane flurry") {}
    bool IsActive() override;
};

// Al'ar <Phoenix God>

class AlarPullingBossTrigger : public Trigger
{
public:
    AlarPullingBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "al'ar pulling boss") {}
    bool IsActive() override;
};

class AlarBossIsFlyingBetweenPlatformsTrigger : public Trigger
{
public:
    AlarBossIsFlyingBetweenPlatformsTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "al'ar boss is flying between platforms") {}
    bool IsActive() override;
};

class AlarEmbersExplodeUponDeathTrigger : public Trigger
{
public:
    AlarEmbersExplodeUponDeathTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "al'ar embers explode upon death") {}
    bool IsActive() override;
};

class AlarKillingEmbersDamagesBossTrigger : public Trigger
{
public:
    AlarKillingEmbersDamagesBossTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "al'ar killing embers damages boss") {}
    bool IsActive() override;
};

class AlarIncomingFlameQuillsTrigger : public Trigger
{
public:
    AlarIncomingFlameQuillsTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "al'ar incoming flame quills") {}
    bool IsActive() override;
};

class AlarRisingFromTheAshesTrigger : public Trigger
{
public:
    AlarRisingFromTheAshesTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "al'ar rising from the ashes") {}
    bool IsActive() override;
};

class AlarIsInPhase2Trigger : public Trigger
{
public:
    AlarIsInPhase2Trigger(PlayerbotAI* botAI)
        : Trigger(botAI, "al'ar is in phase 2") {}
    bool IsActive() override;
};

class AlarShouldManagePhaseTrackerTrigger : public Trigger
{
public:
    AlarShouldManagePhaseTrackerTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "al'ar should manage phase tracker") {}
    bool IsActive() override;
};

// Void Reaver

class VoidReaverShouldBeTankedTrigger : public Trigger
{
public:
    VoidReaverShouldBeTankedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "void reaver should be tanked") {}
    bool IsActive() override;
};

class VoidReaverKnockAwayPullsAggroToNonTanksTrigger : public Trigger
{
public:
    VoidReaverKnockAwayPullsAggroToNonTanksTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "void reaver knock away pulls aggro to non-tanks") {}
    bool IsActive() override;
};

class VoidReaverRangedShouldStandBackTrigger : public Trigger
{
public:
    VoidReaverRangedShouldStandBackTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "void reaver ranged should stand back") {}
    bool IsActive() override;
};

class VoidReaverArcaneOrbIsIncomingTrigger : public Trigger
{
public:
    VoidReaverArcaneOrbIsIncomingTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "void reaver arcane orb is incoming") {}
    bool IsActive() override;
};

// High Astromancer Solarian

class HighAstromancerSolarianShouldBeTankedTrigger : public Trigger
{
public:
    HighAstromancerSolarianShouldBeTankedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "high astromancer solarian should be tanked") {}
    bool IsActive() override;
};

class HighAstromancerSolarianBotHasWrathOfTheAstromancerTrigger : public Trigger
{
public:
    HighAstromancerSolarianBotHasWrathOfTheAstromancerTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "high astromancer solarian bot has wrath of the astromancer") {}
    bool IsActive() override;
};

class HighAstromancerSolarianSolariumPriestsSpawnedTrigger : public Trigger
{
public:
    HighAstromancerSolarianSolariumPriestsSpawnedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "high astromancer solarian solarium priests spawned") {}
    bool IsActive() override;
};

class HighAstromancerSolarianBossCastsPsychicScreamTrigger : public Trigger
{
public:
    HighAstromancerSolarianBossCastsPsychicScreamTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "high astromancer boss casts psychic scream") {}
    bool IsActive() override;
};

// Kael'thas Sunstrider <Lord of the Blood Elves>

class KaelthasSunstriderThaladredIsFixatedOnBotTrigger : public Trigger
{
public:
    KaelthasSunstriderThaladredIsFixatedOnBotTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider thaladred is fixated on bot") {}
    bool IsActive() override;
};

class KaelthasSunstriderPullingTankableAdvisorsTrigger : public Trigger
{
public:
    KaelthasSunstriderPullingTankableAdvisorsTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider pulling tankable advisors") {}
    bool IsActive() override;
};

class KaelthasSunstriderSanguinarOrTelonicusShouldBeTankedTrigger : public Trigger
{
public:
    KaelthasSunstriderSanguinarOrTelonicusShouldBeTankedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider sanguinar or telonicus should be tanked") {}
    bool IsActive() override;
};

class KaelthasSunstriderSanguinarCastsBellowingRoarTrigger : public Trigger
{
public:
    KaelthasSunstriderSanguinarCastsBellowingRoarTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider sanguinar casts bellowing roar") {}
    bool IsActive() override;
};

class KaelthasSunstriderCapernianShouldBeTankedByWarlockTrigger : public Trigger
{
public:
    KaelthasSunstriderCapernianShouldBeTankedByWarlockTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider capernian should be tanked by warlock") {}
    bool IsActive() override;
};

class KaelthasSunstriderCapernianBlowsUpNearAndFarTrigger : public Trigger
{
public:
    KaelthasSunstriderCapernianBlowsUpNearAndFarTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider capernian blows up near and far") {}
    bool IsActive() override;
};

class KaelthasSunstriderBotsShouldHoldPhase3PositionsTrigger : public Trigger
{
public:
    KaelthasSunstriderBotsShouldHoldPhase3PositionsTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider bots should hold phase 3 positions") {}
    bool IsActive() override;
};

class KaelthasSunstriderDeterminingAdvisorKillOrderTrigger : public Trigger
{
public:
    KaelthasSunstriderDeterminingAdvisorKillOrderTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider determining advisor kill order") {}
    bool IsActive() override;
};

class KaelthasSunstriderShouldManageAdvisorDpsTimerTrigger : public Trigger
{
public:
    KaelthasSunstriderShouldManageAdvisorDpsTimerTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider should manage advisor dps timer") {}
    bool IsActive() override;
};

class KaelthasSunstriderLegendaryWeaponsAreAliveTrigger : public Trigger
{
public:
    KaelthasSunstriderLegendaryWeaponsAreAliveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider legendary weapons are alive") {}
    bool IsActive() override;
};

class KaelthasSunstriderLegendaryAxeCastsWhirlwindTrigger : public Trigger
{
public:
    KaelthasSunstriderLegendaryAxeCastsWhirlwindTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider legendary axe casts whirlwind") {}
    bool IsActive() override;
};

class KaelthasSunstriderLegendaryWeaponsAreDeadTrigger : public Trigger
{
public:
    KaelthasSunstriderLegendaryWeaponsAreDeadTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider legendary weapons are dead") {}
    bool IsActive() override;
};

class KaelthasSunstriderLegendaryWeaponsAreEquippedTrigger : public Trigger
{
public:
    KaelthasSunstriderLegendaryWeaponsAreEquippedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider legendary weapons are equipped") {}
    bool IsActive() override;
};

class KaelthasSunstriderLegendaryWeaponsWereLostTrigger : public Trigger
{
public:
    KaelthasSunstriderLegendaryWeaponsWereLostTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider legendary weapons were lost") {}
    bool IsActive() override;
};

class KaelthasSunstriderBossHasEnteredTheFightTrigger : public Trigger
{
public:
    KaelthasSunstriderBossHasEnteredTheFightTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider boss has entered the fight") {}
    bool IsActive() override;
};

class KaelthasSunstriderPhoenixesAndEggsAreSpawningTrigger : public Trigger
{
public:
    KaelthasSunstriderPhoenixesAndEggsAreSpawningTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider phoenixes and eggs are spawning") {}
    bool IsActive() override;
};

class KaelthasSunstriderRaidMemberIsMindControlledTrigger : public Trigger
{
public:
    KaelthasSunstriderRaidMemberIsMindControlledTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider raid member is mind controlled") {}
    bool IsActive() override;
};

class KaelthasSunstriderBossIsManipulatingGravityTrigger : public Trigger
{
public:
    KaelthasSunstriderBossIsManipulatingGravityTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider boss is manipulating gravity") {}
    bool IsActive() override;
};

#endif
