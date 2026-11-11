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
#include <string>

// Shared

class Aq40EncounterTrigger : public Trigger
{
public:
    Aq40EncounterTrigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval)
    {
    }

    bool IsActive() final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, Aq40Helpers::AQ40_MAP_ID) &&
               !Aq40Helpers::HasTrueFulfillment(bot) && IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
    // The bot is free to act: not casting and not escaping a hazard.
    bool IsFreeToAct();
};

class Aq40NoEncounterInProgressTrigger : public Trigger
{
public:
    // Throttled to once per second: between-encounter clean-up has no urgency.
    Aq40NoEncounterInProgressTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 no encounter in progress", 1000) {}
    bool IsActive() override;
};

// Fires for the mechanic tracker bot only, in the given encounter (None: any encounter).
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

// Shared by every hazard: active while the hazard function returns a spot to step to.
class Aq40HazardTrigger : public Aq40EncounterTrigger
{
public:
    Aq40HazardTrigger(PlayerbotAI* botAI, std::string const& name, Aq40Helpers::Aq40GoalFn hazard)
        : Aq40EncounterTrigger(botAI, name), _hazard(hazard)
    {
    }

protected:
    bool IsActiveInEncounter() override;

private:
    Aq40Helpers::Aq40GoalFn const _hazard;
};

// Shared by every encounter position: active while the bot is away from its spot and free to move there.
class Aq40FormationTrigger : public Aq40EncounterTrigger
{
public:
    Aq40FormationTrigger(PlayerbotAI* botAI, std::string const& name, Aq40Helpers::Aq40GoalFn formation,
                         int32 checkInterval = 1)
        : Aq40EncounterTrigger(botAI, name, checkInterval), _formation(formation)
    {
    }

protected:
    bool IsActiveInEncounter() override;

private:
    Aq40Helpers::Aq40GoalFn const _formation;
};

// Shared by each boss's kill order: the bot has an assigned target it is not attacking yet.
class Aq40AssignedTargetTrigger : public Aq40EncounterTrigger
{
public:
    Aq40AssignedTargetTrigger(PlayerbotAI* botAI, std::string const& name, Aq40Helpers::Aq40Encounter encounter)
        : Aq40EncounterTrigger(botAI, name), _encounter(encounter)
    {
    }

protected:
    bool IsActiveInEncounter() override;

private:
    Aq40Helpers::Aq40Encounter const _encounter;
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

// Out of combat: shared by the walks to a pre-pull spot.
class Aq40PrepullTrigger : public Trigger
{
public:
    Aq40PrepullTrigger(PlayerbotAI* botAI, std::string const& name, Aq40Helpers::Aq40GoalFn spot)
        : Trigger(botAI, name, 1), _spot(spot)
    {
    }
    bool IsActive() override;

private:
    Aq40Helpers::Aq40GoalFn const _spot;
};

// The Prophet Skeram

class TheProphetSkeramRaidMemberMindControlledTrigger : public Aq40EncounterTrigger
{
public:
    TheProphetSkeramRaidMemberMindControlledTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "the prophet skeram raid member mind controlled")
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

// Viscidus

class ViscidusShouldBeFrozenTrigger : public Aq40EncounterTrigger
{
public:
    ViscidusShouldBeFrozenTrigger(PlayerbotAI* botAI) : Aq40EncounterTrigger(botAI, "viscidus should be frozen") {}

protected:
    bool IsActiveInEncounter() override;
};

class ViscidusIsFrozenTrigger : public Aq40EncounterTrigger
{
public:
    ViscidusIsFrozenTrigger(PlayerbotAI* botAI) : Aq40EncounterTrigger(botAI, "viscidus is frozen") {}

protected:
    bool IsActiveInEncounter() override;
};

// Princess Huhuran

class PrincessHuhuranFrenziedTrigger : public Aq40EncounterTrigger
{
public:
    PrincessHuhuranFrenziedTrigger(PlayerbotAI* botAI) : Aq40EncounterTrigger(botAI, "princess huhuran frenzied") {}

protected:
    bool IsActiveInEncounter() override;
};

class PrincessHuhuranWyvernStingOnTankTrigger : public Aq40EncounterTrigger
{
public:
    PrincessHuhuranWyvernStingOnTankTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "princess huhuran wyvern sting on tank")
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
        : Aq40EncounterTrigger(botAI, "twin emperors non-tank has emperor aggro")
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class EmperorVeknilashHittingTankTrigger : public Aq40EncounterTrigger
{
public:
    EmperorVeknilashHittingTankTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "emperor vek'nilash hitting tank")
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class TwinEmperorsHealerShouldFocusEmperorTargetsTrigger : public Aq40EncounterTrigger
{
public:
    TwinEmperorsHealerShouldFocusEmperorTargetsTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "twin emperors healer should focus emperor targets")
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class TwinEmperorsWarlockPetActiveTrigger : public Aq40EncounterTrigger
{
public:
    TwinEmperorsWarlockPetActiveTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "twin emperors warlock pet active")
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class TwinEmperorsWarlockWaitingTrigger : public Aq40EncounterTrigger
{
public:
    TwinEmperorsWarlockWaitingTrigger(PlayerbotAI* botAI) : Aq40EncounterTrigger(botAI, "twin emperors warlock waiting")
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class EmperorVeklorHeldByWrongPlayerTrigger : public Aq40EncounterTrigger
{
public:
    EmperorVeklorHeldByWrongPlayerTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "emperor vek'lor held by wrong player")
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class EmperorVeklorShouldBeTankedByWarlockTrigger : public Aq40EncounterTrigger
{
public:
    EmperorVeklorShouldBeTankedByWarlockTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "emperor vek'lor should be tanked by warlock")
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class EmperorVeklorWarlockTankLowHealthTrigger : public Aq40EncounterTrigger
{
public:
    EmperorVeklorWarlockTankLowHealthTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "emperor vek'lor warlock tank low health")
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class EmperorVeklorTankInShoutRangeTrigger : public Aq40EncounterTrigger
{
public:
    EmperorVeklorTankInShoutRangeTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "emperor vek'lor tank in shout range")
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class TwinEmperorsEmperorCannotReachTargetTrigger : public Aq40EncounterTrigger
{
public:
    TwinEmperorsEmperorCannotReachTargetTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "twin emperors emperor cannot reach target")
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

// C'Thun

class CthunMeleeOutOfBurnRangeTrigger : public Aq40EncounterTrigger
{
public:
    CthunMeleeOutOfBurnRangeTrigger(PlayerbotAI* botAI) : Aq40EncounterTrigger(botAI, "c'thun melee out of burn range")
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class CthunHealersShouldDamageCthunTrigger : public Aq40EncounterTrigger
{
public:
    CthunHealersShouldDamageCthunTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "c'thun healers should damage c'thun")
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class CthunHealersShouldDamageFleshTentaclesTrigger : public Aq40EncounterTrigger
{
public:
    CthunHealersShouldDamageFleshTentaclesTrigger(PlayerbotAI* botAI)
        : Aq40EncounterTrigger(botAI, "c'thun healers should damage flesh tentacles")
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class CthunOnSpitOutCircleTrigger : public Aq40EncounterTrigger
{
public:
    CthunOnSpitOutCircleTrigger(PlayerbotAI* botAI) : Aq40EncounterTrigger(botAI, "c'thun on spit-out circle") {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
