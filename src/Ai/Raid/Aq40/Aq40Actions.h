/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AQ40ACTIONS_H
#define PLAYERBOTS_AQ40ACTIONS_H

#include <string>

#include "Action.h"
#include "Aq40Helpers.h"
#include "AttackAction.h"
#include "MovementActions.h"

// Shared

// Marks the strategy's own actions, which the AQ40 multipliers leave alone.
class Aq40ActionTag
{
public:
    virtual ~Aq40ActionTag() = default;
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

class Aq40RangedMoveIntoBossRangeAction : public MovementAction, public Aq40ActionTag
{
public:
    Aq40RangedMoveIntoBossRangeAction(PlayerbotAI* botAI) : MovementAction(botAI, "aq40 ranged move into boss range") {}
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

class TheProphetSkeramMoveToAssignedPlatformAction : public MovementAction, public Aq40ActionTag
{
public:
    TheProphetSkeramMoveToAssignedPlatformAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "the prophet skeram move to assigned platform")
    {
    }
    bool Execute(Event event) override;
};

// Bug Trio

class BugTrioMoveAwayFromPoisonCloudAction : public MovementAction, public Aq40ActionTag
{
public:
    BugTrioMoveAwayFromPoisonCloudAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "bug trio move away from poison cloud")
    {
    }
    bool Execute(Event event) override;
};

class BugTrioTanksSeparateBossesAction : public MovementAction, public Aq40ActionTag
{
public:
    BugTrioTanksSeparateBossesAction(PlayerbotAI* botAI) : MovementAction(botAI, "bug trio tanks separate bosses") {}
    bool Execute(Event event) override;
};

// Battleguard Sartura

class BattleguardSarturaMoveAwayFromWhirlwindAction : public MovementAction, public Aq40ActionTag
{
public:
    BattleguardSarturaMoveAwayFromWhirlwindAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "battleguard sartura move away from whirlwind")
    {
    }
    bool Execute(Event event) override;
};

class BattleguardSarturaMarkKillTargetAction : public Action, public Aq40ActionTag
{
public:
    BattleguardSarturaMarkKillTargetAction(PlayerbotAI* botAI) : Action(botAI, "battleguard sartura mark kill target")
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

class FankrissTheUnyieldingMarkKillTargetAction : public Action
{
public:
    FankrissTheUnyieldingMarkKillTargetAction(PlayerbotAI* botAI)
        : Action(botAI, "fankriss the unyielding mark kill target")
    {
    }
    bool Execute(Event event) override;
};

class FankrissTheUnyieldingMainTankAttackBossAction : public AttackAction
{
public:
    FankrissTheUnyieldingMainTankAttackBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "fankriss the unyielding main tank attack boss")
    {
    }
    bool Execute(Event event) override;
};

class FankrissTheUnyieldingTanksPickUpSpawnsAction : public AttackAction
{
public:
    FankrissTheUnyieldingTanksPickUpSpawnsAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "fankriss the unyielding tanks pick up spawns")
    {
    }
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

class ViscidusEscapeToxinCloudsAction : public MovementAction, public Aq40ActionTag
{
public:
    ViscidusEscapeToxinCloudsAction(PlayerbotAI* botAI) : MovementAction(botAI, "viscidus escape toxin clouds") {}
    bool Execute(Event event) override;
};

class ViscidusTankDragClearOfToxinAction : public MovementAction, public Aq40ActionTag
{
public:
    ViscidusTankDragClearOfToxinAction(PlayerbotAI* botAI) : MovementAction(botAI, "viscidus tank drag clear of toxin")
    {
    }
    bool Execute(Event event) override;

private:
    Position _spot;
    bool _hasSpot = false;
    uint32 _spotTime = 0;
};

class ViscidusMeleeMoveAroundToxinAction : public MovementAction, public Aq40ActionTag
{
public:
    ViscidusMeleeMoveAroundToxinAction(PlayerbotAI* botAI) : MovementAction(botAI, "viscidus melee move around toxin")
    {
    }
    bool Execute(Event event) override;
};

class ViscidusDropCleansingTotemAction : public Action, public Aq40ActionTag
{
public:
    ViscidusDropCleansingTotemAction(PlayerbotAI* botAI) : Action(botAI, "viscidus drop cleansing totem") {}
    bool Execute(Event event) override;
};

class ViscidusApplyFrostOilAction : public Action, public Aq40ActionTag
{
public:
    ViscidusApplyFrostOilAction(PlayerbotAI* botAI) : Action(botAI, "viscidus apply frost oil") {}
    bool Execute(Event event) override;
};

class ViscidusPositionRangedOnRingAction : public MovementAction, public Aq40ActionTag
{
public:
    ViscidusPositionRangedOnRingAction(PlayerbotAI* botAI) : MovementAction(botAI, "viscidus position ranged on ring")
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

class TwinEmperorsWarlockMoveToStartSpotAction : public MovementAction, public Aq40ActionTag
{
public:
    TwinEmperorsWarlockMoveToStartSpotAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "twin emperors warlock move to start spot")
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

class TwinEmperorsMoveAwayFromExplodingBugAction : public MovementAction, public Aq40ActionTag
{
public:
    TwinEmperorsMoveAwayFromExplodingBugAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "twin emperors move away from exploding bug")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsEscapeBlizzardAction : public MovementAction, public Aq40ActionTag
{
public:
    TwinEmperorsEscapeBlizzardAction(PlayerbotAI* botAI) : MovementAction(botAI, "twin emperors escape blizzard") {}
    bool Execute(Event event) override;
};

class TwinEmperorsMoveAwayFromVeklorAction : public MovementAction, public Aq40ActionTag
{
public:
    TwinEmperorsMoveAwayFromVeklorAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "twin emperors move away from vek'lor")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsClearVeknilashLandingSpotAction : public MovementAction, public Aq40ActionTag
{
public:
    TwinEmperorsClearVeknilashLandingSpotAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "twin emperors clear vek'nilash landing spot")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsTanksPositionEmperorsAction : public MovementAction, public Aq40ActionTag
{
public:
    TwinEmperorsTanksPositionEmperorsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "twin emperors tanks position emperors")
    {
    }
    bool Execute(Event event) override;
};

class EmperorVeklorWarlockTankKeepRangeAction : public MovementAction, public Aq40ActionTag
{
public:
    EmperorVeklorWarlockTankKeepRangeAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "emperor vek'lor warlock tank keep range")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsMeleeLeaveLandingSpotsAction : public MovementAction, public Aq40ActionTag
{
public:
    TwinEmperorsMeleeLeaveLandingSpotsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "twin emperors melee leave landing spots")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsDpsMoveToAssignedEmperorAction : public MovementAction, public Aq40ActionTag
{
public:
    TwinEmperorsDpsMoveToAssignedEmperorAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "twin emperors dps move to assigned emperor")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsHealersPositionBetweenTankAndWarlockAction : public MovementAction, public Aq40ActionTag
{
public:
    TwinEmperorsHealersPositionBetweenTankAndWarlockAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "twin emperors healers position between tank and warlock")
    {
    }
    bool Execute(Event event) override;
};

// Ouro

class OuroMoveOutOfFrontalConeAction : public MovementAction, public Aq40ActionTag
{
public:
    OuroMoveOutOfFrontalConeAction(PlayerbotAI* botAI) : MovementAction(botAI, "ouro move out of frontal cone") {}
    bool Execute(Event event) override;
};

class OuroAvoidDirtMoundAction : public MovementAction, public Aq40ActionTag
{
public:
    OuroAvoidDirtMoundAction(PlayerbotAI* botAI) : MovementAction(botAI, "ouro avoid dirt mound") {}
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

class CthunPullerMoveToPullSpotAction : public MovementAction, public Aq40ActionTag
{
public:
    CthunPullerMoveToPullSpotAction(PlayerbotAI* botAI) : MovementAction(botAI, "c'thun puller move to pull spot") {}
    bool Execute(Event event) override;
};

class CthunTrailPullerAction : public MovementAction, public Aq40ActionTag
{
public:
    CthunTrailPullerAction(PlayerbotAI* botAI) : MovementAction(botAI, "c'thun trail puller") {}
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

class CthunExitStomachAction : public MovementAction, public Aq40ActionTag
{
public:
    CthunExitStomachAction(PlayerbotAI* botAI) : MovementAction(botAI, "c'thun exit stomach") {}
    bool Execute(Event event) override;
};

class EyeOfCthunDodgeDarkGlareAction : public MovementAction, public Aq40ActionTag
{
public:
    EyeOfCthunDodgeDarkGlareAction(PlayerbotAI* botAI) : MovementAction(botAI, "eye of c'thun dodge dark glare") {}
    bool Execute(Event event) override;
};

class EyeOfCthunPullerHoldPullSpotAction : public MovementAction, public Aq40ActionTag
{
public:
    EyeOfCthunPullerHoldPullSpotAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "eye of c'thun puller hold pull spot")
    {
    }
    bool Execute(Event event) override;
};

class EyeOfCthunMoveToAssignedSpotAction : public MovementAction, public Aq40ActionTag
{
public:
    EyeOfCthunMoveToAssignedSpotAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "eye of c'thun move to assigned spot")
    {
    }
    bool Execute(Event event) override;
};

class CthunMoveToStackAction : public MovementAction, public Aq40ActionTag
{
public:
    CthunMoveToStackAction(PlayerbotAI* botAI) : MovementAction(botAI, "c'thun move to stack") {}
    bool Execute(Event event) override;
};

class CthunMeleeMoveIntoBurnRangeAction : public MovementAction, public Aq40ActionTag
{
public:
    CthunMeleeMoveIntoBurnRangeAction(PlayerbotAI* botAI) : MovementAction(botAI, "c'thun melee move into burn range")
    {
    }
    bool Execute(Event event) override;
};

#endif
