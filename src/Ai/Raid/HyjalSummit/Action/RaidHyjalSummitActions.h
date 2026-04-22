/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDHYJALSUMMITACTIONS_H
#define _PLAYERBOT_RAIDHYJALSUMMITACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

// General

class HyjalSummitEraseTrackersAction : public Action
{
public:
    HyjalSummitEraseTrackersAction(
        PlayerbotAI* botAI) : Action(botAI, "hyjal summit erase trackers") {}
    bool Execute(Event event) override;
};

// Rage Winterchill

class RageWinterchillMisdirectBossToMainTankAction : public AttackAction
{
public:
    RageWinterchillMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "rage winterchill misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class RageWinterchillMainTankPositionBossAction : public AttackAction
{
public:
    RageWinterchillMainTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "rage winterchill main tank position boss") {}
    bool Execute(Event event) override;
};

class RageWinterchillSpreadRangedInCircleAction : public MovementAction
{
public:
    RageWinterchillSpreadRangedInCircleAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "rage winterchill spread ranged in circle") {}
    bool Execute(Event event) override;
};

class RageWinterchillMeleeGetOutOfDeathAndDecayAction : public AttackAction
{
public:
    RageWinterchillMeleeGetOutOfDeathAndDecayAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "rage winterchill melee get out of death and decay") {}
    bool Execute(Event event) override;
};

// Anetheron

class AnetheronMisdirectBossAndInfernalsToTanksAction : public AttackAction
{
public:
    AnetheronMisdirectBossAndInfernalsToTanksAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "anetheron misdirect boss and infernals to tanks") {}
    bool Execute(Event event) override;
};

class AnetheronMainTankPositionBossAction : public AttackAction
{
public:
    AnetheronMainTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "anetheron main tank position boss") {}
    bool Execute(Event event) override;
};

class AnetheronSpreadRangedInCircleAction : public MovementAction
{
public:
    AnetheronSpreadRangedInCircleAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "anetheron spread ranged in circle") {}
    bool Execute(Event event) override;
};

class AnetheronBringInfernalToInfernalTankAction : public MovementAction
{
public:
    AnetheronBringInfernalToInfernalTankAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "anetheron bring infernal to infernal tank") {}
    bool Execute(Event event) override;
};

class AnetheronFirstAssistTankPickUpInfernalsAction : public AttackAction
{
public:
    AnetheronFirstAssistTankPickUpInfernalsAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "anetheron first assist tank pick up infernals") {}
    bool Execute(Event event) override;
};

class AnetheronAssignDpsPriorityAction : public AttackAction
{
public:
    AnetheronAssignDpsPriorityAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "anetheron assign dps priority") {}
    bool Execute(Event event) override;
};

// Kaz'rogal

class KazrogalMisdirectBossToMainTankAction : public AttackAction
{
public:
    KazrogalMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "kaz'rogal misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class KazrogalMainTankPositionBossAction : public AttackAction
{
public:
    KazrogalMainTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "kaz'rogal main tank position boss") {}
    bool Execute(Event event) override;
};

class KazrogalAssistTanksMoveInFrontOfBossAction : public AttackAction
{
public:
    KazrogalAssistTanksMoveInFrontOfBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "kaz'rogal assist tanks move in front of boss") {}
    bool Execute(Event event) override;
};

class KazrogalSpreadRangedInArcAction : public MovementAction
{
public:
    KazrogalSpreadRangedInArcAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kaz'rogal spread ranged in arc") {}
    bool Execute(Event event) override;
};

class KazrogalLowManaBotTakeDefensiveMeasuresAction : public MovementAction
{
public:
KazrogalLowManaBotTakeDefensiveMeasuresAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kaz'rogal low mana bot take defensive measures") {}
    bool Execute(Event event) override;
};

class KazrogalCastShadowProtectionSpellAction : public Action
{
public:
    KazrogalCastShadowProtectionSpellAction(
        PlayerbotAI* botAI) : Action(botAI, "kaz'rogal cast shadow protection spell") {}
    bool Execute(Event event) override;
};

// Azgalor

class AzgalorMisdirectBossToMainTankAction : public AttackAction
{
public:
    AzgalorMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "azgalor misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class AzgalorMainTankPositionBossAction : public AttackAction
{
public:
    AzgalorMainTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "azgalor main tank position boss") {}
    bool Execute(Event event) override;
};

class AzgalorWaitAtSafePositionAction : public MovementAction
{
public:
    AzgalorWaitAtSafePositionAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "azgalor wait at safe position") {}
    bool Execute(Event event) override;
};

class AzgalorDisperseRangedAction : public MovementAction
{
public:
    AzgalorDisperseRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "azgalor disperse ranged") {}
    bool Execute(Event event) override;
};

class AzgalorMeleeGetOutOfFireAndSwapTargetsAction : public AttackAction
{
public:
    AzgalorMeleeGetOutOfFireAndSwapTargetsAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "azgalor melee get out of fire and swap targets") {}
    bool Execute(Event event) override;
};

class AzgalorMoveToDoomguardTankAction : public MovementAction
{
public:
    AzgalorMoveToDoomguardTankAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "azgalor move to doomguard tank") {}
    bool Execute(Event event) override;
};

class AzgalorFirstAssistTankPositionDoomguardAction : public AttackAction
{
public:
    AzgalorFirstAssistTankPositionDoomguardAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "azgalor first assist tank position doomguard") {}
    bool Execute(Event event) override;
};

class AzgalorRangedDpsPrioritizeDoomguardsAction : public AttackAction
{
public:
    AzgalorRangedDpsPrioritizeDoomguardsAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "azgalor ranged dps prioritize doomguards") {}
    bool Execute(Event event) override;
};

// Archimonde

class ArchimondeMisdirectBossToMainTankAction : public AttackAction
{
public:
    ArchimondeMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "archimonde misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class ArchimondeMoveBossToInitialPositionAction : public AttackAction
{
public:
    ArchimondeMoveBossToInitialPositionAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "archimonde move boss to initial position") {}
    bool Execute(Event event) override;
};

class ArchimondeCastFearImmunitySpellAction : public Action
{
public:
    ArchimondeCastFearImmunitySpellAction(
        PlayerbotAI* botAI) : Action(botAI, "archimonde cast fear immunity spell") {}
    bool Execute(Event event) override;

private:
    bool CastFearWardOnMainTank();
    bool UseTremorTotemStrategy();
};

class ArchimondeSpreadToAvoidAirBurstAction : public MovementAction
{
public:
    ArchimondeSpreadToAvoidAirBurstAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "archimonde spread to avoid air burst") {}
    bool Execute(Event event) override;
};

class ArchimondeAvoidDoomfireAction : public MovementAction
{
public:
    ArchimondeAvoidDoomfireAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "archimonde avoid doomfire") {}
    bool Execute(Event event) override;
};

class ArchimondeRemoveDoomfireDotAction : public Action
{
public:
    ArchimondeRemoveDoomfireDotAction(
        PlayerbotAI* botAI) : Action(botAI, "archimonde remove doomfire dot") {}
    bool Execute(Event event) override;
};

#endif
