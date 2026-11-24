/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KARATRIGGERS_H
#define PLAYERBOTS_KARATRIGGERS_H

#include "EncounterHelpers.h"
#include "KaraHelpers.h"
#include "Trigger.h"
#include <string>

// Shared

class KarazhanEncounterTrigger : public Trigger
{
public:
    KarazhanEncounterTrigger(
        PlayerbotAI* botAI, std::string const name, KaraHelpers::KaraBosses boss,
        int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval), _boss(boss) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsBossInProgress(
            bot, KaraHelpers::KARA_MAP_ID, KaraHelpers::Id(_boss)) && IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
    Unit* GetEncounterBoss() { return KaraHelpers::GetKaraBoss(bot, _boss); }

private:
    KaraHelpers::KaraBosses const _boss;
};

class KarazhanNoEncounterInProgressTrigger : public Trigger
{
public:
    KarazhanNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "karazhan no encounter in progress", 1000) {}
    bool IsActive() override;
};

class KarazhanEnemiesCastFearTrigger : public Trigger
{
public:
    KarazhanEnemiesCastFearTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "karazhan enemies cast fear") {}
    bool IsActive() override;
};

// Trash

class ManaWarpIncomingWarpBreachTrigger : public Trigger
{
public:
    ManaWarpIncomingWarpBreachTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "mana warp incoming warp breach") {}
    bool IsActive() override;
};

class ManaWarpTargetAtLowHealthTrigger : public Trigger
{
public:
    ManaWarpTargetAtLowHealthTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "mana warp target at low health") {}
    bool IsActive() override;
};

// Attumen the Huntsman

class AttumenTheHuntsmanUnmountedShouldBeTankedTrigger : public KarazhanEncounterTrigger
{
public:
    AttumenTheHuntsmanUnmountedShouldBeTankedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "attumen the huntsman unmounted should be tanked",
            KaraHelpers::KaraBosses::DATA_ATTUMEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class AttumenTheHuntsmanMidnightShouldBeFocusedTrigger : public KarazhanEncounterTrigger
{
public:
    AttumenTheHuntsmanMidnightShouldBeFocusedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "attumen the huntsman midnight should be focused",
            KaraHelpers::KaraBosses::DATA_ATTUMEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class AttumenTheHuntsmanMountedShouldBeTankedTrigger : public KarazhanEncounterTrigger
{
public:
    AttumenTheHuntsmanMountedShouldBeTankedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "attumen the huntsman mounted should be tanked",
            KaraHelpers::KaraBosses::DATA_ATTUMEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class AttumenTheHuntsmanShouldStackBehindTrigger : public KarazhanEncounterTrigger
{
public:
    AttumenTheHuntsmanShouldStackBehindTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "attumen the huntsman should stack behind",
            KaraHelpers::KaraBosses::DATA_ATTUMEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class AttumenTheHuntsmanShouldHoldDpsTrigger : public KarazhanEncounterTrigger
{
public:
    AttumenTheHuntsmanShouldHoldDpsTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "attumen the huntsman should hold dps",
            KaraHelpers::KaraBosses::DATA_ATTUMEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class AttumenTheHuntsmanShouldManageDpsTimerTrigger : public KarazhanEncounterTrigger
{
public:
    AttumenTheHuntsmanShouldManageDpsTimerTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "attumen the huntsman should manage dps timer",
            KaraHelpers::KaraBosses::DATA_ATTUMEN) {}

protected:
    bool IsActiveInEncounter() override;
};

// Moroes <Tower Steward>

class MoroesShouldPrioritizeGuestsTrigger : public KarazhanEncounterTrigger
{
public:
    MoroesShouldPrioritizeGuestsTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "moroes should prioritize guests", KaraHelpers::KaraBosses::DATA_MOROES, 1000) {}

protected:
    bool IsActiveInEncounter() override;
};

// Maiden of Virtue

class MaidenOfVirtueShouldBeTankedTrigger : public KarazhanEncounterTrigger
{
public:
    MaidenOfVirtueShouldBeTankedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "maiden of virtue should be tanked", KaraHelpers::KaraBosses::DATA_MAIDEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class MaidenOfVirtueShouldBreakRepentanceTrigger : public KarazhanEncounterTrigger
{
public:
    MaidenOfVirtueShouldBreakRepentanceTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "maiden of virtue should break repentance",
            KaraHelpers::KaraBosses::DATA_MAIDEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class MaidenOfVirtueRangedShouldSpreadTrigger : public KarazhanEncounterTrigger
{
public:
    MaidenOfVirtueRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "maiden of virtue ranged should spread", KaraHelpers::KaraBosses::DATA_MAIDEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class MaidenOfVirtueShamanShouldGroundHolyFireTrigger : public KarazhanEncounterTrigger
{
public:
    MaidenOfVirtueShamanShouldGroundHolyFireTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "maiden of virtue shaman should ground holy fire",
            KaraHelpers::KaraBosses::DATA_MAIDEN) {}

protected:
    bool IsActiveInEncounter() override;
};

// The Big Bad Wolf

class BigBadWolfShouldBeTankedTrigger : public KarazhanEncounterTrigger
{
public:
    BigBadWolfShouldBeTankedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "big bad wolf should be tanked",
            KaraHelpers::KaraBosses::DATA_OPERA_PERFORMANCE) {}

protected:
    bool IsActiveInEncounter() override;
};

class BigBadWolfChasesRedHoodTrigger : public KarazhanEncounterTrigger
{
public:
    BigBadWolfChasesRedHoodTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "big bad wolf chases red hood",
            KaraHelpers::KaraBosses::DATA_OPERA_PERFORMANCE) {}

protected:
    bool IsActiveInEncounter() override;
};

class BigBadWolfVictimHasRedHoodTrigger : public KarazhanEncounterTrigger
{
public:
    BigBadWolfVictimHasRedHoodTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "big bad wolf victim has red hood",
            KaraHelpers::KaraBosses::DATA_OPERA_PERFORMANCE) {}

protected:
    bool IsActiveInEncounter() override;
};

// Romulo and Julianne

class RomuloAndJulianneShouldDieTogetherTrigger : public KarazhanEncounterTrigger
{
public:
    RomuloAndJulianneShouldDieTogetherTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "romulo and julianne should die together",
            KaraHelpers::KaraBosses::DATA_OPERA_PERFORMANCE) {}

protected:
    bool IsActiveInEncounter() override;
};

class JulianneDrinkingPoisonTrigger : public KarazhanEncounterTrigger
{
public:
    JulianneDrinkingPoisonTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "julianne drinking poison",
            KaraHelpers::KaraBosses::DATA_OPERA_PERFORMANCE) {}

protected:
    bool IsActiveInEncounter() override;
};

// The Wizard of Oz

class WizardOfOzShouldPrioritizeTargetsTrigger : public KarazhanEncounterTrigger
{
public:
    WizardOfOzShouldPrioritizeTargetsTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "wizard of oz should prioritize targets",
            KaraHelpers::KaraBosses::DATA_OPERA_PERFORMANCE) {}

protected:
    bool IsActiveInEncounter() override;
};

class WizardOfOzMageShouldBurnStrawmanTrigger : public KarazhanEncounterTrigger
{
public:
    WizardOfOzMageShouldBurnStrawmanTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "wizard of oz mage should burn strawman",
            KaraHelpers::KaraBosses::DATA_OPERA_PERFORMANCE) {}

protected:
    bool IsActiveInEncounter() override;
};

// The Curator

class TheCuratorShouldPrioritizeAstralFlaresTrigger : public KarazhanEncounterTrigger
{
public:
    TheCuratorShouldPrioritizeAstralFlaresTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "the curator should prioritize astral flares",
            KaraHelpers::KaraBosses::DATA_CURATOR) {}

protected:
    bool IsActiveInEncounter() override;
};

class TheCuratorShouldBeTankedTrigger : public KarazhanEncounterTrigger
{
public:
    TheCuratorShouldBeTankedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "the curator should be tanked", KaraHelpers::KaraBosses::DATA_CURATOR) {}

protected:
    bool IsActiveInEncounter() override;
};

class TheCuratorRangedShouldSpreadTrigger : public KarazhanEncounterTrigger
{
public:
    TheCuratorRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "the curator ranged should spread", KaraHelpers::KaraBosses::DATA_CURATOR) {}

protected:
    bool IsActiveInEncounter() override;
};

// Terestian Illhoof

class TerestianIllhoofShouldPrioritizeTargetsTrigger : public KarazhanEncounterTrigger
{
public:
    TerestianIllhoofShouldPrioritizeTargetsTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "terestian illhoof should prioritize targets",
            KaraHelpers::KaraBosses::DATA_TERESTIAN) {}

protected:
    bool IsActiveInEncounter() override;
};

// Shade of Aran

class ShadeOfAranCastingArcaneExplosionTrigger : public KarazhanEncounterTrigger
{
public:
    ShadeOfAranCastingArcaneExplosionTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "shade of aran casting arcane explosion",
            KaraHelpers::KaraBosses::DATA_ARAN) {}

protected:
    bool IsActiveInEncounter() override;
};

class ShadeOfAranRangedInBlizzardTrigger : public KarazhanEncounterTrigger
{
public:
    ShadeOfAranRangedInBlizzardTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "shade of aran ranged in blizzard", KaraHelpers::KaraBosses::DATA_ARAN) {}

protected:
    bool IsActiveInEncounter() override;
};

class ShadeOfAranFlameWreathActiveTrigger : public KarazhanEncounterTrigger
{
public:
    ShadeOfAranFlameWreathActiveTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "shade of aran flame wreath active",
            KaraHelpers::KaraBosses::DATA_ARAN) {}

protected:
    bool IsActiveInEncounter() override;
};

class ShadeOfAranShouldPrioritizeElementalsTrigger : public KarazhanEncounterTrigger
{
public:
    ShadeOfAranShouldPrioritizeElementalsTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "shade of aran should prioritize elementals",
            KaraHelpers::KaraBosses::DATA_ARAN) {}

protected:
    bool IsActiveInEncounter() override;
};

class ShadeOfAranRangedShouldMaintainDistanceTrigger : public KarazhanEncounterTrigger
{
public:
    ShadeOfAranRangedShouldMaintainDistanceTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "shade of aran ranged should maintain distance",
            KaraHelpers::KaraBosses::DATA_ARAN) {}

protected:
    bool IsActiveInEncounter() override;
};

// Netherspite

class NetherspiteShouldBlockRedBeamTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteShouldBlockRedBeamTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "netherspite should block red beam",
            KaraHelpers::KaraBosses::DATA_NETHERSPITE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NetherspiteShouldBlockBlueOrGreenBeamTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteShouldBlockBlueOrGreenBeamTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "netherspite should block blue or green beam",
            KaraHelpers::KaraBosses::DATA_NETHERSPITE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NetherspiteNextBeamBlockerShouldPrepareTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteNextBeamBlockerShouldPrepareTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "netherspite next beam blocker should prepare",
            KaraHelpers::KaraBosses::DATA_NETHERSPITE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NetherspiteShouldStandBetweenBeamsTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteShouldStandBetweenBeamsTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "netherspite should stand between beams",
            KaraHelpers::KaraBosses::DATA_NETHERSPITE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NetherspiteShouldSpreadForNetherbreathTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteShouldSpreadForNetherbreathTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "netherspite should spread for netherbreath",
            KaraHelpers::KaraBosses::DATA_NETHERSPITE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NetherspiteShouldHoldDpsTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteShouldHoldDpsTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "netherspite should hold dps", KaraHelpers::KaraBosses::DATA_NETHERSPITE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NetherspiteShouldManagePetAggressionTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteShouldManagePetAggressionTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "netherspite should manage pet aggression",
            KaraHelpers::KaraBosses::DATA_NETHERSPITE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NetherspiteShouldManageTimersAndTrackersTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteShouldManageTimersAndTrackersTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "netherspite should manage timers and trackers",
            KaraHelpers::KaraBosses::DATA_NETHERSPITE) {}

protected:
    bool IsActiveInEncounter() override;
};

// Prince Malchezaar

class PrinceMalchezaarBotIsEnfeebledTrigger : public KarazhanEncounterTrigger
{
public:
    PrinceMalchezaarBotIsEnfeebledTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "prince malchezaar bot is enfeebled",
            KaraHelpers::KaraBosses::DATA_MALCHEZAAR) {}

protected:
    bool IsActiveInEncounter() override;
};

class PrinceMalchezaarNearHazardTrigger : public KarazhanEncounterTrigger
{
public:
    PrinceMalchezaarNearHazardTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "prince malchezaar near hazard", KaraHelpers::KaraBosses::DATA_MALCHEZAAR) {}

protected:
    bool IsActiveInEncounter() override;
};

class PrinceMalchezaarShouldBeTankedTrigger : public KarazhanEncounterTrigger
{
public:
    PrinceMalchezaarShouldBeTankedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "prince malchezaar should be tanked",
            KaraHelpers::KaraBosses::DATA_MALCHEZAAR) {}

protected:
    bool IsActiveInEncounter() override;
};

// Nightbane

class NightbaneShouldBeTankedTrigger : public KarazhanEncounterTrigger
{
public:
    NightbaneShouldBeTankedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "nightbane should be tanked", KaraHelpers::KaraBosses::DATA_NIGHTBANE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NightbaneRangedShouldAvoidCharredEarthTrigger : public KarazhanEncounterTrigger
{
public:
    NightbaneRangedShouldAvoidCharredEarthTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "nightbane ranged should avoid charred earth",
            KaraHelpers::KaraBosses::DATA_NIGHTBANE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NightbaneMeleeShouldAvoidCharredEarthTrigger : public KarazhanEncounterTrigger
{
public:
    NightbaneMeleeShouldAvoidCharredEarthTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "nightbane melee should avoid charred earth",
            KaraHelpers::KaraBosses::DATA_NIGHTBANE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NightbaneShouldManagePetAggressionTrigger : public KarazhanEncounterTrigger
{
public:
    NightbaneShouldManagePetAggressionTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "nightbane should manage pet aggression",
            KaraHelpers::KaraBosses::DATA_NIGHTBANE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NightbaneShouldHoldDpsTrigger : public KarazhanEncounterTrigger
{
public:
    NightbaneShouldHoldDpsTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "nightbane should hold dps", KaraHelpers::KaraBosses::DATA_NIGHTBANE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NightbaneInFlightPhaseTrigger : public KarazhanEncounterTrigger
{
public:
    NightbaneInFlightPhaseTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "nightbane in flight phase", KaraHelpers::KaraBosses::DATA_NIGHTBANE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NightbaneInLandingPhaseTrigger : public KarazhanEncounterTrigger
{
public:
    NightbaneInLandingPhaseTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "nightbane in landing phase", KaraHelpers::KaraBosses::DATA_NIGHTBANE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NightbaneShouldManageTimersAndTrackersTrigger : public KarazhanEncounterTrigger
{
public:
    NightbaneShouldManageTimersAndTrackersTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "nightbane should manage timers and trackers",
            KaraHelpers::KaraBosses::DATA_NIGHTBANE) {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
