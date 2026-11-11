/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AQ40ACTIONS_H
#define PLAYERBOTS_AQ40ACTIONS_H

#include "Action.h"
#include "Aq40Helpers.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "Position.h"
#include <string>

// Shared

// Marks the strategy's own actions, which the AQ40 multipliers leave alone.
class Aq40ActionTag
{
public:
    virtual ~Aq40ActionTag() = default;
};

class Aq40MovementAction : public MovementAction, public Aq40ActionTag
{
public:
    Aq40MovementAction(PlayerbotAI* botAI, std::string const& name) : MovementAction(botAI, name) {}

protected:
    bool MoveToGoal(Position goal, bool hazard);
    // Before the pull, neither "follow" (drags the bot back to the master) nor "stay" (keeps pulling it back
    // mid-fight) works, so the bot walks to its spot and holds it.
    bool MoveToPrepullSpot(Position const& spot);
};

class Aq40ResetEncounterStatesAction : public Action, public Aq40ActionTag
{
public:
    Aq40ResetEncounterStatesAction(PlayerbotAI* botAI) : Action(botAI, "aq40 reset encounter states") {}
    bool Execute(Event event) override;
};

class Aq40DropToGroundAction : public MovementAction, public Aq40ActionTag
{
public:
    Aq40DropToGroundAction(PlayerbotAI* botAI) : MovementAction(botAI, "aq40 drop to ground") {}
    bool Execute(Event event) override;
};

class Aq40AttackAssignedTargetAction : public AttackAction, public Aq40ActionTag
{
public:
    Aq40AttackAssignedTargetAction(PlayerbotAI* botAI, std::string const& name) : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class Aq40StopAttackingAction : public Action, public Aq40ActionTag
{
public:
    Aq40StopAttackingAction(PlayerbotAI* botAI, std::string const& name) : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class Aq40TankTauntAssignedTargetAction : public Action, public Aq40ActionTag
{
public:
    Aq40TankTauntAssignedTargetAction(PlayerbotAI* botAI) : Action(botAI, "aq40 tank taunt assigned target") {}
    bool Execute(Event event) override;
};

// Tracker actions return false so they never take the bot's turn.
class Aq40TrackEncounterAction : public Action, public Aq40ActionTag
{
public:
    Aq40TrackEncounterAction(PlayerbotAI* botAI) : Action(botAI, "aq40 track encounter") {}
    bool Execute(Event event) override;
};

class Aq40AssignAddTanksAction : public Action, public Aq40ActionTag
{
public:
    Aq40AssignAddTanksAction(PlayerbotAI* botAI) : Action(botAI, "aq40 assign add tanks") {}
    bool Execute(Event event) override;
};

class Aq40RangedMoveIntoBossRangeAction : public Aq40MovementAction
{
public:
    Aq40RangedMoveIntoBossRangeAction(PlayerbotAI* botAI)
        : Aq40MovementAction(botAI, "aq40 ranged move into boss range")
    {
    }
    bool Execute(Event event) override;
};

// The Prophet Skeram

class TheProphetSkeramCrowdControlMindControlledAction : public Action, public Aq40ActionTag
{
public:
    TheProphetSkeramCrowdControlMindControlledAction(PlayerbotAI* botAI)
        : Action(botAI, "the prophet skeram crowd control mind controlled")
    {
    }
    bool Execute(Event event) override;
};

class TheProphetSkeramAssignPlatformsAction : public Action, public Aq40ActionTag
{
public:
    TheProphetSkeramAssignPlatformsAction(PlayerbotAI* botAI) : Action(botAI, "the prophet skeram assign platforms") {}
    bool Execute(Event event) override;
};

class TheProphetSkeramMoveToAssignedPlatformAction : public Aq40MovementAction
{
public:
    TheProphetSkeramMoveToAssignedPlatformAction(PlayerbotAI* botAI)
        : Aq40MovementAction(botAI, "the prophet skeram move to assigned platform")
    {
    }
    bool Execute(Event event) override;
};

// Bug Trio

class BugTrioMoveAwayFromPoisonCloudAction : public Aq40MovementAction
{
public:
    BugTrioMoveAwayFromPoisonCloudAction(PlayerbotAI* botAI)
        : Aq40MovementAction(botAI, "bug trio move away from poison cloud")
    {
    }
    bool Execute(Event event) override;
};

class BugTrioTanksSeparateBossesAction : public Aq40MovementAction
{
public:
    BugTrioTanksSeparateBossesAction(PlayerbotAI* botAI) : Aq40MovementAction(botAI, "bug trio tanks separate bosses")
    {
    }
    bool Execute(Event event) override;
};

// Battleguard Sartura

class BattleguardSarturaMoveAwayFromWhirlwindAction : public Aq40MovementAction
{
public:
    BattleguardSarturaMoveAwayFromWhirlwindAction(PlayerbotAI* botAI)
        : Aq40MovementAction(botAI, "battleguard sartura move away from whirlwind")
    {
    }
    bool Execute(Event event) override;
};

// Fankriss the Unyielding

class FankrissTheUnyieldingSwapTanksAction : public Action, public Aq40ActionTag
{
public:
    FankrissTheUnyieldingSwapTanksAction(PlayerbotAI* botAI) : Action(botAI, "fankriss the unyielding swap tanks") {}
    bool Execute(Event event) override;
};

// Viscidus

class ViscidusCastFrostSpellsAction : public Action, public Aq40ActionTag
{
public:
    ViscidusCastFrostSpellsAction(PlayerbotAI* botAI) : Action(botAI, "viscidus cast frost spells") {}
    bool Execute(Event event) override;
};

class ViscidusMeleeShatterAction : public AttackAction, public Aq40ActionTag
{
public:
    ViscidusMeleeShatterAction(PlayerbotAI* botAI) : AttackAction(botAI, "viscidus melee shatter") {}
    bool Execute(Event event) override;
};

class ViscidusEscapeToxinCloudsAction : public Aq40MovementAction
{
public:
    ViscidusEscapeToxinCloudsAction(PlayerbotAI* botAI) : Aq40MovementAction(botAI, "viscidus escape toxin clouds") {}
    bool Execute(Event event) override;
};

class ViscidusMeleeMoveInToShatterAction : public Aq40MovementAction
{
public:
    ViscidusMeleeMoveInToShatterAction(PlayerbotAI* botAI)
        : Aq40MovementAction(botAI, "viscidus melee move in to shatter")
    {
    }
    bool Execute(Event event) override;
};

class ViscidusPositionRangedOnRingAction : public Aq40MovementAction
{
public:
    ViscidusPositionRangedOnRingAction(PlayerbotAI* botAI)
        : Aq40MovementAction(botAI, "viscidus position ranged on ring")
    {
    }
    bool Execute(Event event) override;
};

// Princess Huhuran

class PrincessHuhuranTranquilizingShotAction : public Action, public Aq40ActionTag
{
public:
    PrincessHuhuranTranquilizingShotAction(PlayerbotAI* botAI) : Action(botAI, "princess huhuran tranquilizing shot") {}
    bool Execute(Event event) override;
};

class PrincessHuhuranCureWyvernStingAction : public Action, public Aq40ActionTag
{
public:
    PrincessHuhuranCureWyvernStingAction(PlayerbotAI* botAI) : Action(botAI, "princess huhuran cure wyvern sting") {}
    bool Execute(Event event) override;
};

class PrincessHuhuranAssignPoisonBoltSoakersAction : public Action, public Aq40ActionTag
{
public:
    PrincessHuhuranAssignPoisonBoltSoakersAction(PlayerbotAI* botAI)
        : Action(botAI, "princess huhuran assign poison bolt soakers")
    {
    }
    bool Execute(Event event) override;
};

class PrincessHuhuranSwapTanksAction : public Action, public Aq40ActionTag
{
public:
    PrincessHuhuranSwapTanksAction(PlayerbotAI* botAI) : Action(botAI, "princess huhuran swap tanks") {}
    bool Execute(Event event) override;
};

class PrincessHuhuranSoakersStandNearBossAction : public MovementAction, public Aq40ActionTag
{
public:
    PrincessHuhuranSoakersStandNearBossAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "princess huhuran soakers stand near boss")
    {
    }
    bool Execute(Event event) override;
};

class PrincessHuhuranSpreadRangedAndHealersAction : public MovementAction, public Aq40ActionTag
{
public:
    PrincessHuhuranSpreadRangedAndHealersAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "princess huhuran spread ranged and healers")
    {
    }
    bool Execute(Event event) override;
};

// Twin Emperors

class TwinEmperorsDropEmperorThreatAction : public Action, public Aq40ActionTag
{
public:
    TwinEmperorsDropEmperorThreatAction(PlayerbotAI* botAI) : Action(botAI, "twin emperors drop emperor threat") {}
    bool Execute(Event event) override;
};

class EmperorVeknilashTankUseDefensiveCooldownAction : public Action, public Aq40ActionTag
{
public:
    EmperorVeknilashTankUseDefensiveCooldownAction(PlayerbotAI* botAI)
        : Action(botAI, "emperor vek'nilash tank use defensive cooldown")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsHealEmperorTargetsAction : public Action, public Aq40ActionTag
{
public:
    TwinEmperorsHealEmperorTargetsAction(PlayerbotAI* botAI) : Action(botAI, "twin emperors heal emperor targets") {}
    bool Execute(Event event) override;
};

class TwinEmperorsWarlockSetPetPassiveAction : public Action, public Aq40ActionTag
{
public:
    TwinEmperorsWarlockSetPetPassiveAction(PlayerbotAI* botAI) : Action(botAI, "twin emperors warlock set pet passive")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsWarlockPrepareForVeklorAction : public Action, public Aq40ActionTag
{
public:
    TwinEmperorsWarlockPrepareForVeklorAction(PlayerbotAI* botAI)
        : Action(botAI, "twin emperors warlock prepare for vek'lor")
    {
    }
    bool Execute(Event event) override;
};

class EmperorVeklorWaitingTankTauntBossAction : public Action, public Aq40ActionTag
{
public:
    EmperorVeklorWaitingTankTauntBossAction(PlayerbotAI* botAI)
        : Action(botAI, "emperor vek'lor waiting tank taunt boss")
    {
    }
    bool Execute(Event event) override;
};

class EmperorVeklorWarlockTankCastSearingPainAction : public Action, public Aq40ActionTag
{
public:
    EmperorVeklorWarlockTankCastSearingPainAction(PlayerbotAI* botAI)
        : Action(botAI, "emperor vek'lor warlock tank cast searing pain")
    {
    }
    bool Execute(Event event) override;
};

class EmperorVeklorWarlockVoidwalkerSacrificeAction : public Action, public Aq40ActionTag
{
public:
    EmperorVeklorWarlockVoidwalkerSacrificeAction(PlayerbotAI* botAI)
        : Action(botAI, "emperor vek'lor warlock voidwalker sacrifice")
    {
    }
    bool Execute(Event event) override;
};

class EmperorVeklorTankDemoralizingShoutAction : public Action, public Aq40ActionTag
{
public:
    EmperorVeklorTankDemoralizingShoutAction(PlayerbotAI* botAI)
        : Action(botAI, "emperor vek'lor tank demoralizing shout")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsLeadEmperorToPlatformAction : public MovementAction, public Aq40ActionTag
{
public:
    TwinEmperorsLeadEmperorToPlatformAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "twin emperors lead emperor to platform")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsWarlockMoveToStartSpotAction : public Aq40MovementAction
{
public:
    TwinEmperorsWarlockMoveToStartSpotAction(PlayerbotAI* botAI)
        : Aq40MovementAction(botAI, "twin emperors warlock move to start spot")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsAssignTanksAndSidesAction : public Action, public Aq40ActionTag
{
public:
    TwinEmperorsAssignTanksAndSidesAction(PlayerbotAI* botAI) : Action(botAI, "twin emperors assign tanks and sides") {}
    bool Execute(Event event) override;
};

class TwinEmperorsMoveAwayFromExplodingBugAction : public Aq40MovementAction
{
public:
    TwinEmperorsMoveAwayFromExplodingBugAction(PlayerbotAI* botAI)
        : Aq40MovementAction(botAI, "twin emperors move away from exploding bug")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsEscapeBlizzardAction : public Aq40MovementAction
{
public:
    TwinEmperorsEscapeBlizzardAction(PlayerbotAI* botAI) : Aq40MovementAction(botAI, "twin emperors escape blizzard") {}
    bool Execute(Event event) override;
};

class TwinEmperorsMoveAwayFromVeklorAction : public Aq40MovementAction
{
public:
    TwinEmperorsMoveAwayFromVeklorAction(PlayerbotAI* botAI)
        : Aq40MovementAction(botAI, "twin emperors move away from vek'lor")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsClearVeknilashLandingSpotAction : public Aq40MovementAction
{
public:
    TwinEmperorsClearVeknilashLandingSpotAction(PlayerbotAI* botAI)
        : Aq40MovementAction(botAI, "twin emperors clear vek'nilash landing spot")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsTanksPositionEmperorsAction : public Aq40MovementAction
{
public:
    TwinEmperorsTanksPositionEmperorsAction(PlayerbotAI* botAI)
        : Aq40MovementAction(botAI, "twin emperors tanks position emperors")
    {
    }
    bool Execute(Event event) override;
};

class EmperorVeklorWarlockTankKeepRangeAction : public Aq40MovementAction
{
public:
    EmperorVeklorWarlockTankKeepRangeAction(PlayerbotAI* botAI)
        : Aq40MovementAction(botAI, "emperor vek'lor warlock tank keep range")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsMeleeLeaveLandingSpotsAction : public Aq40MovementAction
{
public:
    TwinEmperorsMeleeLeaveLandingSpotsAction(PlayerbotAI* botAI)
        : Aq40MovementAction(botAI, "twin emperors melee leave landing spots")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsDpsMoveToAssignedEmperorAction : public Aq40MovementAction
{
public:
    TwinEmperorsDpsMoveToAssignedEmperorAction(PlayerbotAI* botAI)
        : Aq40MovementAction(botAI, "twin emperors dps move to assigned emperor")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsHealersPositionBetweenTankAndWarlockAction : public Aq40MovementAction
{
public:
    TwinEmperorsHealersPositionBetweenTankAndWarlockAction(PlayerbotAI* botAI)
        : Aq40MovementAction(botAI, "twin emperors healers position between tank and warlock")
    {
    }
    bool Execute(Event event) override;
};

// Ouro

class OuroMoveOutOfFrontalConeAction : public Aq40MovementAction
{
public:
    OuroMoveOutOfFrontalConeAction(PlayerbotAI* botAI) : Aq40MovementAction(botAI, "ouro move out of frontal cone") {}
    bool Execute(Event event) override;
};

class OuroAvoidDirtMoundAction : public Aq40MovementAction
{
public:
    OuroAvoidDirtMoundAction(PlayerbotAI* botAI) : Aq40MovementAction(botAI, "ouro avoid dirt mound") {}
    bool Execute(Event event) override;
};

class OuroAcceptNewMainTankAction : public Action, public Aq40ActionTag
{
public:
    OuroAcceptNewMainTankAction(PlayerbotAI* botAI) : Action(botAI, "ouro accept new main tank") {}
    bool Execute(Event event) override;
};

// C'Thun

class CthunHealersDamageCthunAction : public Action, public Aq40ActionTag
{
public:
    CthunHealersDamageCthunAction(PlayerbotAI* botAI) : Action(botAI, "c'thun healers damage c'thun") {}
    bool Execute(Event event) override;
};

class CthunHealersDamageFleshTentaclesAction : public Action, public Aq40ActionTag
{
public:
    CthunHealersDamageFleshTentaclesAction(PlayerbotAI* botAI) : Action(botAI, "c'thun healers damage flesh tentacles")
    {
    }
    bool Execute(Event event) override;
};

class CthunSendSpitOutTriggerAction : public Action, public Aq40ActionTag
{
public:
    CthunSendSpitOutTriggerAction(PlayerbotAI* botAI) : Action(botAI, "c'thun send spit-out trigger") {}
    bool Execute(Event event) override;
};

class CthunPullerMoveToPullSpotAction : public Aq40MovementAction
{
public:
    CthunPullerMoveToPullSpotAction(PlayerbotAI* botAI) : Aq40MovementAction(botAI, "c'thun puller move to pull spot")
    {
    }
    bool Execute(Event event) override;
};

class CthunTrailPullerAction : public Aq40MovementAction
{
public:
    CthunTrailPullerAction(PlayerbotAI* botAI) : Aq40MovementAction(botAI, "c'thun trail puller") {}
    bool Execute(Event event) override;
};

class EyeOfCthunTrackDarkGlareAction : public Action, public Aq40ActionTag
{
public:
    EyeOfCthunTrackDarkGlareAction(PlayerbotAI* botAI) : Action(botAI, "eye of c'thun track dark glare") {}
    bool Execute(Event event) override;
};

class CthunAssignSpotsAction : public Action, public Aq40ActionTag
{
public:
    CthunAssignSpotsAction(PlayerbotAI* botAI) : Action(botAI, "c'thun assign spots") {}
    bool Execute(Event event) override;
};

class CthunAssignTentacleKillersAction : public Action, public Aq40ActionTag
{
public:
    CthunAssignTentacleKillersAction(PlayerbotAI* botAI) : Action(botAI, "c'thun assign tentacle killers") {}
    bool Execute(Event event) override;
};

class CthunAssignGiantClawTanksAction : public Action, public Aq40ActionTag
{
public:
    CthunAssignGiantClawTanksAction(PlayerbotAI* botAI) : Action(botAI, "c'thun assign giant claw tanks") {}
    bool Execute(Event event) override;
};

class CthunExitStomachAction : public Aq40MovementAction
{
public:
    CthunExitStomachAction(PlayerbotAI* botAI) : Aq40MovementAction(botAI, "c'thun exit stomach") {}
    bool Execute(Event event) override;
};

class EyeOfCthunDodgeDarkGlareAction : public Aq40MovementAction
{
public:
    EyeOfCthunDodgeDarkGlareAction(PlayerbotAI* botAI) : Aq40MovementAction(botAI, "eye of c'thun dodge dark glare") {}
    bool Execute(Event event) override;
};

class EyeOfCthunPullerHoldPullSpotAction : public Aq40MovementAction
{
public:
    EyeOfCthunPullerHoldPullSpotAction(PlayerbotAI* botAI)
        : Aq40MovementAction(botAI, "eye of c'thun puller hold pull spot")
    {
    }
    bool Execute(Event event) override;
};

class EyeOfCthunMoveToAssignedSpotAction : public Aq40MovementAction
{
public:
    EyeOfCthunMoveToAssignedSpotAction(PlayerbotAI* botAI)
        : Aq40MovementAction(botAI, "eye of c'thun move to assigned spot")
    {
    }
    bool Execute(Event event) override;
};

class CthunMoveToStackAction : public Aq40MovementAction
{
public:
    CthunMoveToStackAction(PlayerbotAI* botAI) : Aq40MovementAction(botAI, "c'thun move to stack") {}
    bool Execute(Event event) override;
};

class CthunMeleeMoveIntoBurnRangeAction : public Aq40MovementAction
{
public:
    CthunMeleeMoveIntoBurnRangeAction(PlayerbotAI* botAI)
        : Aq40MovementAction(botAI, "c'thun melee move into burn range")
    {
    }
    bool Execute(Event event) override;
};

#endif
