/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AQ40TRIGGERS_H
#define PLAYERBOTS_AQ40TRIGGERS_H

#include "Aq40Helpers.h"
#include "EncounterHelpers.h"
#include "Trigger.h"
#include <optional>
#include <string>

// Shared

class Aq40EncounterTrigger : public Trigger
{
public:
    Aq40EncounterTrigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval)
    {
    }
    Aq40EncounterTrigger(PlayerbotAI* botAI, std::string const name, Aq40Helpers::Aq40Encounter encounter,
                         int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval), _encounter(encounter)
    {
    }

    bool IsActive() final
    {
        bool const inProgress = _encounter ? Aq40Helpers::IsInEncounter(bot, *_encounter)
                                           : EncounterHelpers::IsEncounterInProgress(bot, Aq40Helpers::AQ40_MAP_ID);
        return inProgress && !Aq40Helpers::HasTrueFulfillment(bot) && IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
    bool IsFreeToAct();

private:
    std::optional<Aq40Helpers::Aq40Encounter> const _encounter;
};

class Aq40NoEncounterInProgressTrigger : public Trigger
{
public:
    // Throttled to once per second: between-encounter clean-up has no urgency.
    Aq40NoEncounterInProgressTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 no encounter in progress", 1000) {}
    bool IsActive() override;
};

class Aq40TrackerTrigger : public Aq40EncounterTrigger
{
public:
    Aq40TrackerTrigger(PlayerbotAI* botAI, std::string const& name, Aq40Helpers::Aq40Encounter encounter)
        : Aq40EncounterTrigger(botAI, name, Aq40Helpers::TRACKER_INTERVAL_MS), _encounter(encounter)
    {
    }

protected:
    bool IsActiveInEncounter() override;

private:
    Aq40Helpers::Aq40Encounter const _encounter;
};

class Aq40StuckMidairTrigger : public Aq40EncounterTrigger
{
public:
    Aq40StuckMidairTrigger(PlayerbotAI* botAI) : Aq40EncounterTrigger(botAI, "aq40 stuck midair after knockback") {}

protected:
    bool IsActiveInEncounter() override;
};

class Aq40AssignedTargetTrigger : public Aq40EncounterTrigger
{
public:
    Aq40AssignedTargetTrigger(PlayerbotAI* botAI, std::string const& name, Aq40Helpers::Aq40Encounter encounter)
        : Aq40EncounterTrigger(botAI, name, encounter)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class Aq40NoAssignedTargetTrigger : public Aq40EncounterTrigger
{
public:
    Aq40NoAssignedTargetTrigger(PlayerbotAI* botAI) : Aq40EncounterTrigger(botAI, "aq40 no assigned target") {}

protected:
    bool IsActiveInEncounter() override;
};

class Aq40TankShouldTauntTrigger : public Aq40EncounterTrigger
{
public:
    Aq40TankShouldTauntTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "aq40 tank should taunt assigned target")
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class Aq40RangedShouldHoldBossRangeTrigger : public Aq40EncounterTrigger
{
public:
    Aq40RangedShouldHoldBossRangeTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "aq40 ranged should hold boss range", Aq40Helpers::POSITION_INTERVAL_MS)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

// The Prophet Skeram

class TheProphetSkeramRaidMemberMindControlledTrigger : public Aq40EncounterTrigger
{
public:
    TheProphetSkeramRaidMemberMindControlledTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "the prophet skeram raid member mind controlled",
                               Aq40Helpers::Aq40Encounter::Skeram)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class TheProphetSkeramShouldMoveToPlatformTrigger : public Aq40EncounterTrigger
{
public:
    TheProphetSkeramShouldMoveToPlatformTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "the prophet skeram should move to platform", Aq40Helpers::Aq40Encounter::Skeram)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

// Bug Trio

class BugTrioPoisonCloudNearbyTrigger : public Aq40EncounterTrigger
{
public:
    BugTrioPoisonCloudNearbyTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "bug trio poison cloud nearby", Aq40Helpers::Aq40Encounter::Trio)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class BugTrioBossesTooCloseTrigger : public Aq40EncounterTrigger
{
public:
    BugTrioBossesTooCloseTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "bug trio bosses too close", Aq40Helpers::Aq40Encounter::Trio,
                               Aq40Helpers::POSITION_INTERVAL_MS)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

// Battleguard Sartura

class BattleguardSarturaWhirlwindNearbyTrigger : public Aq40EncounterTrigger
{
public:
    BattleguardSarturaWhirlwindNearbyTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "battleguard sartura whirlwind nearby", Aq40Helpers::Aq40Encounter::Sartura)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

// Fankriss the Unyielding

class FankrissTheUnyieldingBossShouldBeTankedTrigger : public Aq40EncounterTrigger
{
public:
    FankrissTheUnyieldingBossShouldBeTankedTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "fankriss the unyielding boss should be tanked",
                               Aq40Helpers::Aq40Encounter::Fankriss)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class FankrissTheUnyieldingSpawnShouldBeTankedTrigger : public Aq40EncounterTrigger
{
public:
    FankrissTheUnyieldingSpawnShouldBeTankedTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "fankriss the unyielding spawn should be tanked",
                               Aq40Helpers::Aq40Encounter::Fankriss)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

// Viscidus

class ViscidusShouldBeFrozenTrigger : public Aq40EncounterTrigger
{
public:
    ViscidusShouldBeFrozenTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "viscidus should be frozen", Aq40Helpers::Aq40Encounter::Viscidus)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class ViscidusIsFrozenTrigger : public Aq40EncounterTrigger
{
public:
    ViscidusIsFrozenTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "viscidus is frozen", Aq40Helpers::Aq40Encounter::Viscidus)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class ViscidusToxinCloudNearbyTrigger : public Aq40EncounterTrigger
{
public:
    ViscidusToxinCloudNearbyTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "viscidus toxin cloud nearby", Aq40Helpers::Aq40Encounter::Viscidus)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class ViscidusFrozenOutOfMeleeRangeTrigger : public Aq40EncounterTrigger
{
public:
    ViscidusFrozenOutOfMeleeRangeTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "viscidus frozen out of melee range", Aq40Helpers::Aq40Encounter::Viscidus)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class ViscidusRangedShouldHoldRingTrigger : public Aq40EncounterTrigger
{
public:
    ViscidusRangedShouldHoldRingTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "viscidus ranged should hold ring", Aq40Helpers::Aq40Encounter::Viscidus,
                               Aq40Helpers::POSITION_INTERVAL_MS)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

// Princess Huhuran

class PrincessHuhuranFrenziedTrigger : public Aq40EncounterTrigger
{
public:
    PrincessHuhuranFrenziedTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "princess huhuran frenzied", Aq40Helpers::Aq40Encounter::Huhuran)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class PrincessHuhuranWyvernStingOnTankTrigger : public Aq40EncounterTrigger
{
public:
    PrincessHuhuranWyvernStingOnTankTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "princess huhuran wyvern sting on tank", Aq40Helpers::Aq40Encounter::Huhuran)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class PrincessHuhuranSoakerTooFarTrigger : public Aq40EncounterTrigger
{
public:
    PrincessHuhuranSoakerTooFarTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "princess huhuran soaker too far", Aq40Helpers::Aq40Encounter::Huhuran,
                               Aq40Helpers::POSITION_INTERVAL_MS)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class PrincessHuhuranRangedShouldSpreadTrigger : public Aq40EncounterTrigger
{
public:
    PrincessHuhuranRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "princess huhuran ranged should spread", Aq40Helpers::Aq40Encounter::Huhuran,
                               Aq40Helpers::POSITION_INTERVAL_MS)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

// Twin Emperors

class TwinEmperorsNonTankHasEmperorAggroTrigger : public Aq40EncounterTrigger
{
public:
    TwinEmperorsNonTankHasEmperorAggroTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "twin emperors non-tank has emperor aggro", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class EmperorVeknilashHittingTankTrigger : public Aq40EncounterTrigger
{
public:
    EmperorVeknilashHittingTankTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "emperor vek'nilash hitting tank", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class TwinEmperorsHealerShouldFocusEmperorTargetsTrigger : public Aq40EncounterTrigger
{
public:
    TwinEmperorsHealerShouldFocusEmperorTargetsTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "twin emperors healer should focus emperor targets",
                               Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class TwinEmperorsWarlockPetActiveTrigger : public Aq40EncounterTrigger
{
public:
    TwinEmperorsWarlockPetActiveTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "twin emperors warlock pet active", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class TwinEmperorsWarlockWaitingTrigger : public Aq40EncounterTrigger
{
public:
    TwinEmperorsWarlockWaitingTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "twin emperors warlock waiting", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class EmperorVeklorHeldByWrongPlayerTrigger : public Aq40EncounterTrigger
{
public:
    EmperorVeklorHeldByWrongPlayerTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "emperor vek'lor held by wrong player", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class EmperorVeklorShouldBeTankedByWarlockTrigger : public Aq40EncounterTrigger
{
public:
    EmperorVeklorShouldBeTankedByWarlockTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "emperor vek'lor should be tanked by warlock", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class EmperorVeklorWarlockTankLowHealthTrigger : public Aq40EncounterTrigger
{
public:
    EmperorVeklorWarlockTankLowHealthTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "emperor vek'lor warlock tank low health", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class EmperorVeklorTankInShoutRangeTrigger : public Aq40EncounterTrigger
{
public:
    EmperorVeklorTankInShoutRangeTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "emperor vek'lor tank in shout range", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class TwinEmperorsEmperorCannotReachTargetTrigger : public Aq40EncounterTrigger
{
public:
    TwinEmperorsEmperorCannotReachTargetTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "twin emperors emperor cannot reach target", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class TwinEmperorsWarlockShouldTakeStartSpotTrigger : public Trigger
{
public:
    TwinEmperorsWarlockShouldTakeStartSpotTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "twin emperors warlock should take start spot")
    {
    }
    bool IsActive() override;
};

class TwinEmperorsExplodingBugNearbyTrigger : public Aq40EncounterTrigger
{
public:
    TwinEmperorsExplodingBugNearbyTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "twin emperors exploding bug nearby", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class TwinEmperorsInBlizzardTrigger : public Aq40EncounterTrigger
{
public:
    TwinEmperorsInBlizzardTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "twin emperors in blizzard", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class EmperorVeklorArcaneBurstRangeTrigger : public Aq40EncounterTrigger
{
public:
    EmperorVeklorArcaneBurstRangeTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "emperor vek'lor arcane burst range", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class EmperorVeknilashChoosingTargetAfterTeleportTrigger : public Aq40EncounterTrigger
{
public:
    EmperorVeknilashChoosingTargetAfterTeleportTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "emperor vek'nilash choosing target after teleport",
                               Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class TwinEmperorsTanksShouldHoldPlatformsTrigger : public Aq40EncounterTrigger
{
public:
    TwinEmperorsTanksShouldHoldPlatformsTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "twin emperors tanks should hold platforms", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class EmperorVeklorWarlockTankOutOfRangeTrigger : public Aq40EncounterTrigger
{
public:
    EmperorVeklorWarlockTankOutOfRangeTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "emperor vek'lor warlock tank out of range", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class TwinEmperorsTeleportImminentTrigger : public Aq40EncounterTrigger
{
public:
    TwinEmperorsTeleportImminentTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "twin emperors teleport imminent", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class TwinEmperorsDpsShouldPositionTrigger : public Aq40EncounterTrigger
{
public:
    TwinEmperorsDpsShouldPositionTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "twin emperors dps should position", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class TwinEmperorsHealersShouldPositionTrigger : public Aq40EncounterTrigger
{
public:
    TwinEmperorsHealersShouldPositionTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "twin emperors healers should position", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

// Ouro

class OuroFrontalConeTrigger : public Aq40EncounterTrigger
{
public:
    OuroFrontalConeTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "ouro frontal cone", Aq40Helpers::Aq40Encounter::Ouro)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class OuroDirtMoundNearbyTrigger : public Aq40EncounterTrigger
{
public:
    OuroDirtMoundNearbyTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "ouro dirt mound nearby", Aq40Helpers::Aq40Encounter::Ouro)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

// C'Thun

class CthunShouldAssignDpsPriorityTrigger : public Aq40AssignedTargetTrigger
{
public:
    CthunShouldAssignDpsPriorityTrigger(PlayerbotAI* botAI)
        : Aq40AssignedTargetTrigger(botAI, "c'thun should assign dps priority", Aq40Helpers::Aq40Encounter::Cthun)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class CthunMeleeOutOfBurnRangeTrigger : public Aq40EncounterTrigger
{
public:
    CthunMeleeOutOfBurnRangeTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "c'thun melee out of burn range", Aq40Helpers::Aq40Encounter::Cthun)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class CthunHealersShouldDamageCthunTrigger : public Aq40EncounterTrigger
{
public:
    CthunHealersShouldDamageCthunTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "c'thun healers should damage c'thun", Aq40Helpers::Aq40Encounter::Cthun)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class CthunHealersShouldDamageFleshTentaclesTrigger : public Aq40EncounterTrigger
{
public:
    CthunHealersShouldDamageFleshTentaclesTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "c'thun healers should damage flesh tentacles", Aq40Helpers::Aq40Encounter::Cthun)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class CthunOnSpitOutCircleTrigger : public Aq40EncounterTrigger
{
public:
    CthunOnSpitOutCircleTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "c'thun on spit-out circle", Aq40Helpers::Aq40Encounter::Cthun)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class CthunPullerShouldWalkInTrigger : public Trigger
{
public:
    CthunPullerShouldWalkInTrigger(PlayerbotAI* botAI) : Trigger(botAI, "c'thun puller should walk in") {}
    bool IsActive() override;
};

class CthunRaidShouldTrailPullerTrigger : public Trigger
{
public:
    CthunRaidShouldTrailPullerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "c'thun raid should trail puller") {}
    bool IsActive() override;
};

class CthunShouldLeaveStomachTrigger : public Aq40EncounterTrigger
{
public:
    CthunShouldLeaveStomachTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "c'thun should leave stomach", Aq40Helpers::Aq40Encounter::Cthun)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class EyeOfCthunDarkGlareIncomingTrigger : public Aq40EncounterTrigger
{
public:
    EyeOfCthunDarkGlareIncomingTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "eye of c'thun dark glare incoming", Aq40Helpers::Aq40Encounter::Cthun)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class EyeOfCthunPullerHoldingTrigger : public Aq40EncounterTrigger
{
public:
    EyeOfCthunPullerHoldingTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "eye of c'thun puller holding", Aq40Helpers::Aq40Encounter::Cthun)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class EyeOfCthunOutOfAssignedSpotTrigger : public Aq40EncounterTrigger
{
public:
    EyeOfCthunOutOfAssignedSpotTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "eye of c'thun out of assigned spot", Aq40Helpers::Aq40Encounter::Cthun)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class CthunOutOfStackTrigger : public Aq40EncounterTrigger
{
public:
    CthunOutOfStackTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "c'thun out of stack", Aq40Helpers::Aq40Encounter::Cthun)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class CthunMeleeShouldCloseInForBurnTrigger : public Aq40EncounterTrigger
{
public:
    CthunMeleeShouldCloseInForBurnTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "c'thun melee should close in for burn", Aq40Helpers::Aq40Encounter::Cthun)
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

#endif
