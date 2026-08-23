/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TKACTIONS_H
#define PLAYERBOTS_TKACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "TKHelpers.h"
#include <utility>
#include <vector>

using namespace TkHelpers;

// General

class TempestKeepResetEncounterStatesAction : public Action
{
public:
    TempestKeepResetEncounterStatesAction(PlayerbotAI* botAI)
        : Action(botAI, "tempest keep reset encounter states") {}
    bool Execute(Event event) override;
};

class TempestKeepCastFearWardOnMainTankAction : public Action
{
public:
    TempestKeepCastFearWardOnMainTankAction(PlayerbotAI* botAI)
        : Action(botAI, "tempest keep cast fear ward on main tank") {}
    bool Execute(Event event) override;
};

// Trash

class CrimsonHandCenturionCastPolymorphAction : public Action
{
public:
    CrimsonHandCenturionCastPolymorphAction(PlayerbotAI* botAI)
        : Action(botAI, "crimson hand centurion cast polymorph") {}
    bool Execute(Event event) override;
};

// Al'ar <Phoenix God>

class AlarMisdirectBossToMainTankAction : public Action
{
public:
    AlarMisdirectBossToMainTankAction(PlayerbotAI* botAI)
        : Action(botAI, "al'ar misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class AlarBossTanksMoveBetweenPlatformsAction : public AttackAction
{
public:
    AlarBossTanksMoveBetweenPlatformsAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "al'ar boss tanks move between platforms") {}
    bool Execute(Event event) override;
};

class AlarMeleeDpsMoveBetweenPlatformsAction : public AttackAction
{
public:
    AlarMeleeDpsMoveBetweenPlatformsAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "al'ar melee dps move between platforms") {}
    bool Execute(Event event) override;
};

class AlarRangedAndEmberTankMoveUnderPlatformsAction : public AttackAction
{
public:
    AlarRangedAndEmberTankMoveUnderPlatformsAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "al'ar ranged and ember tank move under platforms") {}
    bool Execute(Event event) override;
};

class AlarAssistTanksPickUpEmbersAction : public AttackAction
{
public:
    AlarAssistTanksPickUpEmbersAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "al'ar assist tanks pick up embers") {}
    bool Execute(Event event) override;

private:
    bool HandlePhase1Embers(Unit* alar);
    bool HandlePhase2Embers(Event const& event);
};

class AlarRangedDpsPrioritizeEmbersAction : public AttackAction
{
public:
    AlarRangedDpsPrioritizeEmbersAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "al'ar ranged dps prioritize embers") {}
    bool Execute(Event event) override;
};

class AlarJumpFromPlatformAction : public MovementAction
{
public:
    AlarJumpFromPlatformAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "al'ar jump from platform") {}
    bool Execute(Event event) override;
};

class AlarMoveAwayFromRebirthAction : public MovementAction
{
public:
    AlarMoveAwayFromRebirthAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "al'ar move away from rebirth") {}
    bool Execute(Event event) override;
};

class AlarSwapTanksOnBossAction : public AttackAction
{
public:
    AlarSwapTanksOnBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "al'ar swap tanks on boss") {}
    bool Execute(Event event) override;
};

class AlarAvoidFlamePatchesAndDiveBombsAction : public MovementAction
{
public:
    AlarAvoidFlamePatchesAndDiveBombsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "al'ar avoid flame patches and dive bombs") {}
    bool Execute(Event event) override;

private:
    bool AvoidFlamePatch();
    bool HandleDiveBomb(Unit* alar);
};

class AlarManagePhaseTrackerAction : public Action
{
public:
    AlarManagePhaseTrackerAction(PlayerbotAI* botAI)
        : Action(botAI, "al'ar manage phase tracker") {}
    bool Execute(Event event) override;
};

// Void Reaver

class VoidReaverTanksPositionBossAction : public AttackAction
{
public:
    VoidReaverTanksPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "void reaver tanks position boss") {}
    bool Execute(Event event) override;
};

class VoidReaverUseAggroDumpAbilityAction : public Action
{
public:
    VoidReaverUseAggroDumpAbilityAction(PlayerbotAI* botAI)
        : Action(botAI, "void reaver use aggro dump ability") {}
    bool Execute(Event event) override;
};

class VoidReaverKeepRangedInGoldilocksZoneAction : public MovementAction
{
public:
    VoidReaverKeepRangedInGoldilocksZoneAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "void reaver keep ranged in goldilocks zone") {}
    bool Execute(Event event) override;
};

class VoidReaverAvoidArcaneOrbAction : public MovementAction
{
public:
    VoidReaverAvoidArcaneOrbAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "void reaver avoid arcane orb") {}
    bool Execute(Event event) override;
};

// High Astromancer Solarian

class HighAstromancerSolarianMainTankPickUpBossAction : public AttackAction
{
public:
    HighAstromancerSolarianMainTankPickUpBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "high astromancer solarian main tank pick up boss") {}
    bool Execute(Event event) override;
};

class HighAstromancerSolarianMoveAwayFromGroupAction : public MovementAction
{
public:
    HighAstromancerSolarianMoveAwayFromGroupAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "high astromancer solarian move away from group") {}
    bool Execute(Event event) override;
};

class HighAstromancerSolarianTargetSolariumPriestsAction : public AttackAction
{
public:
    HighAstromancerSolarianTargetSolariumPriestsAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "high astromancer solarian target solarium priests") {}
    bool Execute(Event event) override;

private:
    std::vector<Player*> GetMeleeBots();
    Unit* AssignSolariumPriestsToMeleeBots(
        std::pair<Unit*, Unit*> const& priestsPair, std::vector<Player*> const& meleeMembers);
};

// Kael'thas Sunstrider <Lord of the Blood Elves>

class KaelthasSunstriderKiteThaladredAction : public MovementAction
{
public:
    KaelthasSunstriderKiteThaladredAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kael'thas sunstrider kite thaladred") {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderMisdirectAdvisorsToTanksAction : public Action
{
public:
    KaelthasSunstriderMisdirectAdvisorsToTanksAction(PlayerbotAI* botAI)
        : Action(botAI, "kael'thas sunstrider misdirect advisors to tanks") {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderMeleeTanksPositionAdvisorsAction : public AttackAction
{
public:
    KaelthasSunstriderMeleeTanksPositionAdvisorsAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "kael'thas sunstrider melee tanks position advisors") {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderWarlockTankPositionCapernianAction : public AttackAction
{
public:
    KaelthasSunstriderWarlockTankPositionCapernianAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "kael'thas sunstrider warlock tank position capernian") {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction : public MovementAction
{
public:
    KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kael'thas sunstrider spread and move away from capernian") {}
    bool Execute(Event event) override;

private:
    bool RangedBotsDisperse(uint32 phase, Unit* capernian);
    bool MeleeStayBackFromCapernian(Unit* capernian);
};

class KaelthasSunstriderHandleAdvisorRolesInPhase3Action : public MovementAction
{
public:
    KaelthasSunstriderHandleAdvisorRolesInPhase3Action(PlayerbotAI* botAI)
        : MovementAction(botAI, "kael'thas sunstrider handle advisor roles in phase 3") {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderAssignAdvisorDpsPriorityAction : public AttackAction
{
public:
    KaelthasSunstriderAssignAdvisorDpsPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "kael'thas sunstrider assign advisor dps priority") {}
    bool Execute(Event event) override;

private:
    bool MeleeDpsPositionOutsideBombRange(Unit* telonicus);
};

class KaelthasSunstriderManageAdvisorDpsTimerAction : public Action
{
public:
    KaelthasSunstriderManageAdvisorDpsTimerAction(PlayerbotAI* botAI)
        : Action(botAI, "kael'thas sunstrider manage advisor dps timer") {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderAssignLegendaryWeaponDpsPriorityAction : public AttackAction
{
public:
    KaelthasSunstriderAssignLegendaryWeaponDpsPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "kael'thas sunstrider assign legendary weapon dps priority") {}
    bool Execute(Event event) override;

private:
    bool HandleDevastationAvoidance(
        Unit* axe, Unit* mace, Unit* dagger, Unit* sword, bool isTank, bool isMeleeDps);
};

class KaelthasSunstriderMoveDevastationAwayAction : public AttackAction
{
public:
    KaelthasSunstriderMoveDevastationAwayAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "kael'thas sunstrider move devastation away") {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderLootLegendaryWeaponsAction : public MovementAction
{
public:
    KaelthasSunstriderLootLegendaryWeaponsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kael'thas sunstrider loot legendary weapons") {}
    bool Execute(Event event) override;

private:
    struct WeaponInfo
    {
        TkNpcs npcEntry;
        TkItems itemId;
    };
    bool ShouldBotLootWeapon(TkNpcs weaponEntry);
    bool LootWeapon(uint32 weaponEntry, uint32 itemId);
    bool EquipLegendaryWeapon(uint32 itemId);
};

class KaelthasSunstriderUseLegendaryWeaponsAction : public Action
{
public:
    KaelthasSunstriderUseLegendaryWeaponsAction(PlayerbotAI* botAI)
        : Action(botAI, "kael'thas sunstrider use legendary weapons") {}
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
    KaelthasSunstriderReequipGearAction(PlayerbotAI* botAI)
        : Action(botAI, "kael'thas sunstrider reequip gear") {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderMainTankPositionBossAction : public AttackAction
{
public:
    KaelthasSunstriderMainTankPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "kael'thas sunstrider main tank position boss") {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderAvoidFlameStrikeAction : public MovementAction
{
public:
    KaelthasSunstriderAvoidFlameStrikeAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kael'thas sunstrider avoid flame strike") {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderHandlePhoenixesAndEggsAction : public AttackAction
{
public:
    KaelthasSunstriderHandlePhoenixesAndEggsAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "kael'thas sunstrider handle phoenixes and eggs") {}
    bool Execute(Event event) override;

private:
    bool AssistTanksPickUpPhoenixes();
    bool NonTanksDestroyEggsAndAvoidPhoenixes();
};

class KaelthasSunstriderBreakMindControlAction : public AttackAction
{
public:
    KaelthasSunstriderBreakMindControlAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "kael'thas sunstrider break mind control") {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderSpreadOutInMidairAction : public MovementAction
{
public:
    KaelthasSunstriderSpreadOutInMidairAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kael'thas sunstrider spread out in midair") {}
    bool Execute(Event event) override;

private:
    bool DropToGround();
    bool HoverAndSpread();
};

#endif
