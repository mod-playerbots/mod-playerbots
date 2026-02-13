#ifndef _PLAYERBOT_RAIDSSCACTIONS_H
#define _PLAYERBOT_RAIDSSCACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

// General

class SerpentShrineCavernEraseTimersAndTrackersAction : public Action
{
public:
    SerpentShrineCavernEraseTimersAndTrackersAction(
        PlayerbotAI* botAI, std::string const name = "serpent shrine cavern erase timers and trackers") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Trash

class UnderbogColossusEscapeToxicPoolAction : public MovementAction
{
public:
    UnderbogColossusEscapeToxicPoolAction(
        PlayerbotAI* botAI, std::string const name = "underbog colossus escape toxic pool") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class GreyheartTidecallerMarkWaterElementalTotemAction : public Action
{
public:
    GreyheartTidecallerMarkWaterElementalTotemAction(
        PlayerbotAI* botAI, std::string const name = "greyheart tidecaller mark water elemental totem") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Hydross the Unstable <Duke of Currents>

class HydrossTheUnstablePositionFrostTankAction : public AttackAction
{
public:
    HydrossTheUnstablePositionFrostTankAction(
        PlayerbotAI* botAI, std::string const name = "hydross the unstable position frost tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class HydrossTheUnstablePositionNatureTankAction : public AttackAction
{
public:
    HydrossTheUnstablePositionNatureTankAction(
        PlayerbotAI* botAI, std::string const name = "hydross the unstable position nature tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class HydrossTheUnstablePrioritizeElementalAddsAction : public AttackAction
{
public:
    HydrossTheUnstablePrioritizeElementalAddsAction(
        PlayerbotAI* botAI, std::string const name = "hydross the unstable prioritize elemental adds") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class HydrossTheUnstableFrostPhaseSpreadOutAction : public MovementAction
{
public:
    HydrossTheUnstableFrostPhaseSpreadOutAction(
        PlayerbotAI* botAI, std::string const name = "hydross the unstable frost phase spread out") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class HydrossTheUnstableMisdirectBossToTankAction : public Action
{
public:
    HydrossTheUnstableMisdirectBossToTankAction(
        PlayerbotAI* botAI, std::string const name = "hydross the unstable misdirect boss to tank") : Action(botAI, name) {}
    bool Execute(Event event) override;

private:
    bool TryMisdirectToFrostTank(Unit* hydross, Group* group);
    bool TryMisdirectToNatureTank(Unit* hydross, Group* group);
};

class HydrossTheUnstableStopDpsUponPhaseChangeAction : public Action
{
public:
    HydrossTheUnstableStopDpsUponPhaseChangeAction(
        PlayerbotAI* botAI, std::string const name = "hydross the unstable stop dps upon phase change") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class HydrossTheUnstableManageTimersAction : public Action
{
public:
    HydrossTheUnstableManageTimersAction(
        PlayerbotAI* botAI, std::string const name = "hydross the unstable manage timers") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// The Lurker Below

class TheLurkerBelowRunAroundBehindBossAction : public MovementAction
{
public:
    TheLurkerBelowRunAroundBehindBossAction(
        PlayerbotAI* botAI, std::string const name = "the lurker below run around behind boss") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class TheLurkerBelowPositionMainTankAction : public AttackAction
{
public:
    TheLurkerBelowPositionMainTankAction(
        PlayerbotAI* botAI, std::string const name = "the lurker below position main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class TheLurkerBelowSpreadRangedInArcAction : public MovementAction
{
public:
    TheLurkerBelowSpreadRangedInArcAction(
        PlayerbotAI* botAI, std::string const name = "the lurker below spread ranged in arc") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class TheLurkerBelowTanksPickUpAddsAction : public AttackAction
{
public:
    TheLurkerBelowTanksPickUpAddsAction(
        PlayerbotAI* botAI, std::string const name = "the lurker below tanks pick up adds") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class TheLurkerBelowManageSpoutTimerAction : public Action
{
public:
    TheLurkerBelowManageSpoutTimerAction(
        PlayerbotAI* botAI, std::string const name = "the lurker below manage spout timer") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Leotheras the Blind

class LeotherasTheBlindTargetSpellbindersAction : public Action
{
public:
    LeotherasTheBlindTargetSpellbindersAction(
        PlayerbotAI* botAI, std::string const name = "leotheras the blind target spellbinders") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindPositionRangedAction : public MovementAction
{
public:
    LeotherasTheBlindPositionRangedAction(
        PlayerbotAI* botAI, std::string const name = "leotheras the blind position ranged") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindDemonFormTankAttackBossAction : public AttackAction
{
public:
    LeotherasTheBlindDemonFormTankAttackBossAction(
        PlayerbotAI* botAI, std::string const name = "leotheras the blind demon form tank attack boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindMeleeTanksDontAttackDemonFormAction : public Action
{
public:
    LeotherasTheBlindMeleeTanksDontAttackDemonFormAction(
        PlayerbotAI* botAI, std::string const name = "leotheras the blind melee tanks don't attack demon form") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindRunAwayFromWhirlwindAction : public MovementAction
{
public:
    LeotherasTheBlindRunAwayFromWhirlwindAction(
        PlayerbotAI* botAI, std::string const name = "leotheras the blind run away from whirlwind") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindMeleeDpsRunAwayFromBossAction : public MovementAction
{
public:
    LeotherasTheBlindMeleeDpsRunAwayFromBossAction(
        PlayerbotAI* botAI, std::string const name = "leotheras the blind melee dps run away from boss") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindDestroyInnerDemonAction : public AttackAction
{
public:
    LeotherasTheBlindDestroyInnerDemonAction(
        PlayerbotAI* botAI, std::string const name = "leotheras the blind destroy inner demon") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    bool HandleFeralTankStrategy(Unit* innerDemon);
    bool HandleHealerStrategy(Unit* innerDemon);
};

class LeotherasTheBlindFinalPhaseAssignDpsPriorityAction : public AttackAction
{
public:
    LeotherasTheBlindFinalPhaseAssignDpsPriorityAction(
        PlayerbotAI* botAI, std::string const name = "leotheras the blind final phase assign dps priority") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindMisdirectBossToDemonFormTankAction : public AttackAction
{
public:
    LeotherasTheBlindMisdirectBossToDemonFormTankAction(
        PlayerbotAI* botAI, std::string const name = "leotheras the blind misdirect boss to demon form tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindManageDpsWaitTimersAction : public Action
{
public:
    LeotherasTheBlindManageDpsWaitTimersAction(
        PlayerbotAI* botAI, std::string const name = "leotheras the blind manage dps wait timers") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Fathom-Lord Karathress

class FathomLordKarathressMainTankPositionBossAction : public AttackAction
{
public:
    FathomLordKarathressMainTankPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "fathom-lord karathress main tank position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FathomLordKarathressFirstAssistTankPositionCaribdisAction : public AttackAction
{
public:
    FathomLordKarathressFirstAssistTankPositionCaribdisAction(
        PlayerbotAI* botAI, std::string const name = "fathom-lord karathress first assist tank position caribdis") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FathomLordKarathressSecondAssistTankPositionSharkkisAction : public AttackAction
{
public:
    FathomLordKarathressSecondAssistTankPositionSharkkisAction(
        PlayerbotAI* botAI, std::string const name = "fathom-lord karathress second assist tank position sharkkis") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FathomLordKarathressThirdAssistTankPositionTidalvessAction : public AttackAction
{
public:
    FathomLordKarathressThirdAssistTankPositionTidalvessAction(
        PlayerbotAI* botAI, std::string const name = "fathom-lord karathress third assist tank position tidalvess") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FathomLordKarathressPositionCaribdisTankHealerAction : public MovementAction
{
public:
    FathomLordKarathressPositionCaribdisTankHealerAction(
        PlayerbotAI* botAI, std::string const name = "fathom-lord karathress position caribdis tank healer") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FathomLordKarathressMisdirectBossesToTanksAction : public AttackAction
{
public:
    FathomLordKarathressMisdirectBossesToTanksAction(
        PlayerbotAI* botAI, std::string const name = "fathom-lord karathress misdirect bosses to tanks") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FathomLordKarathressAssignDpsPriorityAction : public AttackAction
{
public:
    FathomLordKarathressAssignDpsPriorityAction(
        PlayerbotAI* botAI, std::string const name = "fathom-lord karathress assign dps priority") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FathomLordKarathressManageDpsTimerAction : public Action
{
public:
    FathomLordKarathressManageDpsTimerAction(
        PlayerbotAI* botAI, std::string const name = "fathom-lord karathress manage dps timer") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Morogrim Tidewalker

class MorogrimTidewalkerMisdirectBossToMainTankAction : public AttackAction
{
public:
    MorogrimTidewalkerMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const name = "morogrim tidewalker misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class MorogrimTidewalkerMoveBossToTankPositionAction : public AttackAction
{
public:
    MorogrimTidewalkerMoveBossToTankPositionAction(
        PlayerbotAI* botAI, std::string const name = "morogrim tidewalker move boss to tank position") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    bool MoveToPhase1TankPosition(Unit* tidewalker);
    bool MoveToPhase2TankPosition(Unit* tidewalker);
};

class MorogrimTidewalkerPhase2RepositionRangedAction : public MovementAction
{
public:
    MorogrimTidewalkerPhase2RepositionRangedAction(
        PlayerbotAI* botAI, std::string const name = "morogrim tidewalker phase 2 reposition ranged") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

// Lady Vashj <Coilfang Matron>

class LadyVashjMainTankPositionBossAction : public AttackAction
{
public:
    LadyVashjMainTankPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "lady vashj main tank position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjPhase1SpreadRangedInArcAction : public MovementAction
{
public:
    LadyVashjPhase1SpreadRangedInArcAction(
        PlayerbotAI* botAI, std::string const name = "lady vashj phase 1 spread ranged in arc") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjSetGroundingTotemInMainTankGroupAction : public MovementAction
{
public:
    LadyVashjSetGroundingTotemInMainTankGroupAction(
        PlayerbotAI* botAI, std::string const name = "lady vashj set grounding totem in main tank group") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjStaticChargeMoveAwayFromGroupAction : public MovementAction
{
public:
    LadyVashjStaticChargeMoveAwayFromGroupAction(
        PlayerbotAI* botAI, std::string const name = "lady vashj static charge move away from group") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjMisdirectBossToMainTankAction : public AttackAction
{
public:
    LadyVashjMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const name = "lady vashj misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjAssignPhase2AndPhase3DpsPriorityAction : public AttackAction
{
public:
    LadyVashjAssignPhase2AndPhase3DpsPriorityAction(
        PlayerbotAI* botAI, std::string const name = "lady vashj assign phase 2 and phase 3 dps priority") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjMisdirectStriderToFirstAssistTankAction : public AttackAction
{
public:
    LadyVashjMisdirectStriderToFirstAssistTankAction(
        PlayerbotAI* botAI, std::string const name = "lady vashj misdirect strider to first assist tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjTankAttackAndMoveAwayStriderAction : public AttackAction
{
public:
    LadyVashjTankAttackAndMoveAwayStriderAction(
        PlayerbotAI* botAI, std::string const name = "lady vashj tank attack and move away strider") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjTeleportToTaintedElementalAction : public AttackAction
{
public:
    LadyVashjTeleportToTaintedElementalAction(
        PlayerbotAI* botAI, std::string const name = "lady vashj teleport to tainted elemental") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjLootTaintedCoreAction : public MovementAction
{
public:
    LadyVashjLootTaintedCoreAction(
        PlayerbotAI* botAI, std::string const name = "lady vashj loot tainted core") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjPassTheTaintedCoreAction : public MovementAction
{
public:
    LadyVashjPassTheTaintedCoreAction(
        PlayerbotAI* botAI, std::string const name = "lady vashj pass the tainted core") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    bool LineUpFirstCorePasser(Player* designatedLooter, Unit* closestTrigger);
    bool LineUpSecondCorePasser(Player* firstCorePasser, Unit* closestTrigger);
    bool LineUpThirdCorePasser(Player* designatedLooter, Player* firstCorePasser, Player* secondCorePasser, Unit* closestTrigger);
    bool LineUpFourthCorePasser(Player* firstCorePasser, Player* secondCorePasser, Player* thirdCorePasser, Unit* closestTrigger);
    bool IsFirstCorePasserInIntendedPosition(Player* designatedLooter, Player* firstCorePasser, Unit* closestTrigger);
    bool IsSecondCorePasserInIntendedPosition(Player* firstCorePasser, Player* secondCorePasser, Unit* closestTrigger);
    bool IsThirdCorePasserInIntendedPosition(Player* secondCorePasser, Player* thirdCorePasser, Unit* closestTrigger);
    bool IsFourthCorePasserInIntendedPosition(Player* thirdCorePasser, Player* fourthCorePasser, Unit* closestTrigger);
    void ScheduleTransferCoreAfterImbue(PlayerbotAI* botAI, Player* giver, Player* receiver);
    bool UseCoreOnNearestGenerator(const uint32 instanceId);
};

class LadyVashjDestroyTaintedCoreAction : public Action
{
public:
    LadyVashjDestroyTaintedCoreAction(PlayerbotAI* botAI, std::string const name = "lady vashj destroy tainted core") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjEraseCorePassingTrackersAction : public Action
{
public:
    LadyVashjEraseCorePassingTrackersAction(PlayerbotAI* botAI, std::string const name = "lady vashj erase core passing trackers") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjAvoidToxicSporesAction : public MovementAction
{
public:
    LadyVashjAvoidToxicSporesAction(PlayerbotAI* botAI, std::string const name = "lady vashj avoid toxic spores") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
    static std::vector<Unit*> GetAllSporeDropTriggers(PlayerbotAI* botAI, Player* bot);

private:
    Position FindSafestNearbyPosition(const std::vector<Unit*>& spores, const Position& position, float maxRadius, float hazardRadius);
    bool IsPathSafeFromSpores(const Position& start, const Position& end, const std::vector<Unit*>& spores, float hazardRadius);
};

class LadyVashjUseFreeActionAbilitiesAction : public Action
{
public:
    LadyVashjUseFreeActionAbilitiesAction(PlayerbotAI* botAI, std::string const name = "lady vashj use free action abilities") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
