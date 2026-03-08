#ifndef _PLAYERBOT_RAIDTEMPESTKEEPACTIONS_H
#define _PLAYERBOT_RAIDTEMPESTKEEPACTIONS_H

#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepKaelthasBossAI.h"
#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

using namespace TempestKeepHelpers;

// Trash

class CrimsonHandCenturionCastPolymorphAction : public Action
{
public:
    CrimsonHandCenturionCastPolymorphAction(
        PlayerbotAI* botAI, std::string const name = "crimson hand centurion cast polymorph") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Al'ar <Phoenix God>

class AlarMisdirectBossToMainTankAction : public AttackAction
{
public:
    AlarMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const name = "al'ar misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AlarBossTanksMoveBetweenPlatformsAction : public AttackAction
{
public:
    AlarBossTanksMoveBetweenPlatformsAction(
        PlayerbotAI* botAI, std::string const name = "al'ar boss tanks move between platforms") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    bool PositionMainTank(Unit* alar, int8 locationIndex);
    bool PositionAssistTank(Unit* alar, int8 locationIndex);
};

class AlarMeleeDpsMoveBetweenPlatformsAction : public AttackAction
{
public:
    AlarMeleeDpsMoveBetweenPlatformsAction(
        PlayerbotAI* botAI, std::string const name = "al'ar melee dps move between platforms") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AlarRangedAndEmberTankMoveUnderPlatformsAction : public AttackAction
{
public:
    AlarRangedAndEmberTankMoveUnderPlatformsAction(
        PlayerbotAI* botAI, std::string const name = "al'ar ranged and ember tank move under platforms") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AlarAssistTanksPickUpEmbersAction : public AttackAction
{
public:
    AlarAssistTanksPickUpEmbersAction(
        PlayerbotAI* botAI, std::string const name = "al'ar assist tanks pick up embers") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    bool HandlePhase1Embers(Unit* alar);
    bool HandlePhase2Embers(Unit* alar);
};

class AlarRangedDpsPrioritizeEmbersAction : public AttackAction
{
public:
    AlarRangedDpsPrioritizeEmbersAction(
        PlayerbotAI* botAI, std::string const name = "al'ar ranged dps prioritize embers") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AlarJumpFromPlatformAction : public MovementAction
{
public:
    AlarJumpFromPlatformAction(
        PlayerbotAI* botAI, std::string const name = "al'ar jump from platform") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AlarMoveAwayFromRebirthAction : public MovementAction
{
public:
    AlarMoveAwayFromRebirthAction(
        PlayerbotAI* botAI, std::string const name = "al'ar move away from rebirth") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AlarSwapTanksOnBossAction : public AttackAction
{
public:
    AlarSwapTanksOnBossAction(
        PlayerbotAI* botAI, std::string const name = "al'ar swap tanks on boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AlarAvoidFlamePatchesAndDiveBombsAction : public MovementAction
{
public:
    AlarAvoidFlamePatchesAndDiveBombsAction(
        PlayerbotAI* botAI, std::string const name = "al'ar avoid flame patches and dive bombs") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    bool AvoidFlamePatch();
    bool HandleDiveBomb(Unit* alar);
};

class AlarReturnToRoomCenterAction : public MovementAction
{
public:
    AlarReturnToRoomCenterAction(
        PlayerbotAI* botAI, std::string const name = "al'ar return to room center") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AlarManagePhaseTrackerAction : public Action
{
public:
    AlarManagePhaseTrackerAction(
        PlayerbotAI* botAI, std::string const name = "al'ar manage phase tracker") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Void Reaver

class VoidReaverTanksPositionBossAction : public AttackAction
{
public:
    VoidReaverTanksPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "void reaver tanks position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class VoidReaverUseAggroDumpAbilityAction : public Action
{
public:
    VoidReaverUseAggroDumpAbilityAction(
        PlayerbotAI* botAI, std::string const name = "void reaver use aggro dump ability") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class VoidReaverSpreadRangedAction : public MovementAction
{
public:
    VoidReaverSpreadRangedAction(
        PlayerbotAI* botAI, std::string const name = "void reaver spread ranged") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    int GetHealerIndex(Group* group, int& healerCount);
    int GetRangedDpsIndex(Group* group, int& rangedDpsCount);
};

class VoidReaverAvoidArcaneOrbAction : public MovementAction
{
public:
    VoidReaverAvoidArcaneOrbAction(
        PlayerbotAI* botAI, std::string const name = "void reaver avoid arcane orb") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class VoidReaverEraseTrackersAction : public Action
{
public:
    VoidReaverEraseTrackersAction(
        PlayerbotAI* botAI, std::string const name = "void reaver erase trackers") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// High Astromancer Solarian

class HighAstromancerSolarianRangedLeaveSpaceForMeleeAction : public MovementAction
{
public:
    HighAstromancerSolarianRangedLeaveSpaceForMeleeAction(
        PlayerbotAI* botAI, std::string const name = "high astromancer solarian ranged leave space for melee") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class HighAstromancerSolarianMoveAwayFromGroupAction : public MovementAction
{
public:
    HighAstromancerSolarianMoveAwayFromGroupAction(
        PlayerbotAI* botAI, std::string const name = "high astromancer solarian move away from group") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class HighAstromancerSolarianStackForAoeAction : public MovementAction
{
public:
    HighAstromancerSolarianStackForAoeAction(
        PlayerbotAI* botAI, std::string const name = "high astromancer solarian stack for aoe") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class HighAstromancerSolarianTargetSolariumPriestsAction : public AttackAction
{
public:
    HighAstromancerSolarianTargetSolariumPriestsAction(
        PlayerbotAI* botAI, std::string const name = "high astromancer solarian target solarium priests") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    std::pair<Unit*, Unit*> GetSolariumPriests(PlayerbotAI* botAI);
    std::vector<Player*> GetMeleeBots(Group* group);
    Unit* AssignSolariumPriestsToBots(const std::pair<Unit*, Unit*>& priestsPair, const std::vector<Player*>& meleeMembers);
};

class HighAstromancerSolarianCastFearWardOnMainTankAction : public Action
{
public:
    HighAstromancerSolarianCastFearWardOnMainTankAction(
        PlayerbotAI* botAI, std::string const name = "high astromancer solarian cast fear ward on main tank") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Kael'thas Sunstrider <Lord of the Blood Elves>

class KaelthasSunstriderKiteThaladredAction : public MovementAction
{
public:
    KaelthasSunstriderKiteThaladredAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kael'thas sunstrider kite thaladred") {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderMisdirectAdvisorsToTanksAction : public AttackAction
{
public:
    KaelthasSunstriderMisdirectAdvisorsToTanksAction(
        PlayerbotAI* botAI, std::string const name = "kael'thas sunstrider misdirect advisors to tanks") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderMainTankPositionSanguinarAction : public AttackAction
{
public:
    KaelthasSunstriderMainTankPositionSanguinarAction(
        PlayerbotAI* botAI, std::string const name = "kael'thas sunstrider main tank position sanguinar") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderCastFearWardOnSanguinarTankAction : public Action
{
public:
    KaelthasSunstriderCastFearWardOnSanguinarTankAction(
        PlayerbotAI* botAI, std::string const name = "kael'thas sunstrider cast fear ward on sanguinar tank") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderWarlockTankPositionCapernianAction : public AttackAction
{
public:
    KaelthasSunstriderWarlockTankPositionCapernianAction(
        PlayerbotAI* botAI, std::string const name = "kael'thas sunstrider warlock tank position capernian") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction : public MovementAction
{
public:
    KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction(
        PlayerbotAI* botAI, std::string const name = "kael'thas sunstrider spread and move away from capernian") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    bool RangedBotsDisperse(boss_kaelthas* kaelAI, Unit* capernian);
    bool MeleeStayBackFromCapernian(Unit* capernian);
};

class KaelthasSunstriderFirstAssistTankPositionTelonicusAction : public AttackAction
{
public:
    KaelthasSunstriderFirstAssistTankPositionTelonicusAction(
        PlayerbotAI* botAI, std::string const name = "kael'thas sunstrider first assist tank position telonicus") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderHandleAdvisorRolesInPhase3Action : public MovementAction
{
public:
    KaelthasSunstriderHandleAdvisorRolesInPhase3Action(
        PlayerbotAI* botAI, std::string const name = "kael'thas sunstrider handle advisor roles in phase 3") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderAssignAdvisorDpsPriorityAction : public AttackAction
{
public:
    KaelthasSunstriderAssignAdvisorDpsPriorityAction(
        PlayerbotAI* botAI, std::string const name = "kael'thas sunstrider assign advisor dps priority") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderManageAdvisorDpsTimerAction : public Action
{
public:
    KaelthasSunstriderManageAdvisorDpsTimerAction(
        PlayerbotAI* botAI, std::string const name = "kael'thas sunstrider manage advisor dps timer") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderAssignLegendaryWeaponDpsPriorityAction : public AttackAction
{
public:
    KaelthasSunstriderAssignLegendaryWeaponDpsPriorityAction(
        PlayerbotAI* botAI, std::string const name = "kael'thas sunstrider assign legendary weapon dps priority") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderMoveDevastationAwayAction : public AttackAction
{
public:
    KaelthasSunstriderMoveDevastationAwayAction(
        PlayerbotAI* botAI, std::string const name = "kael'thas sunstrider move devastation away") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderLootLegendaryWeaponsAction : public MovementAction
{
public:
    KaelthasSunstriderLootLegendaryWeaponsAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kael'thas sunstrider loot legendary weapons") {}
    bool Execute(Event event) override;

private:
    bool ShouldBotLootWeapon(uint32 weaponEntry);
    bool LootWeapon(uint32 weaponEntry, uint32 itemId);
};

class KaelthasSunstriderUseLegendaryWeaponsAction : public Action
{
public:
    KaelthasSunstriderUseLegendaryWeaponsAction(
        PlayerbotAI* botAI) : Action(botAI, "kael'thas sunstrider use legendary weapons") {}
    bool Execute(Event event) override;

private:
    bool UsePhaseshiftBulwark();
    bool UseStaffOfDisintegration();
    bool UseNetherstrandLongbow();
    bool UseEquippedItemWithPacket(Item* item);
};

class KaelthasSunstriderReequipGearAction : public Action
{
public:
    KaelthasSunstriderReequipGearAction(
        PlayerbotAI* botAI) : Action(botAI, "kael'thas sunstrider reequip gear") {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderMainTankPositionBossAction : public AttackAction
{
public:
    KaelthasSunstriderMainTankPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "kael'thas sunstrider main tank position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderAvoidFlameStrikeAction : public MovementAction
{
public:
    KaelthasSunstriderAvoidFlameStrikeAction(
        PlayerbotAI* botAI, std::string const name = "kael'thas sunstrider avoid flame strike") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderHandlePhoenixesAndEggsAction : public AttackAction
{
public:
    KaelthasSunstriderHandlePhoenixesAndEggsAction(
        PlayerbotAI* botAI, std::string const name = "kael'thas sunstrider handle phoenixes and eggs") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    bool AssistTanksPickUpPhoenixes();
    bool NonTanksDestroyEggsAndAvoidPhoenixes();
};

class KaelthasSunstriderBreakMindControlAction : public AttackAction
{
public:
    KaelthasSunstriderBreakMindControlAction(
        PlayerbotAI* botAI, std::string const name = "kael'thas sunstrider break mind control") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderBreakThroughShockBarrierAction : public AttackAction
{
public:
    KaelthasSunstriderBreakThroughShockBarrierAction(
        PlayerbotAI* botAI, std::string const name = "kael'thas sunstrider break through shock barrier") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderSpreadOutInMidairAction : public MovementAction
{
public:
    KaelthasSunstriderSpreadOutInMidairAction(
        PlayerbotAI* botAI, std::string const name = "kael'thas sunstrider spread out in midair") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
