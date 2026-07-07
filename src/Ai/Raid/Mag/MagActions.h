/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MAGACTIONS_H
#define PLAYERBOTS_MAGACTIONS_H

#include "MagHelpers.h"
#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class MagtheridonMainTankAttackFirstThreeChannelersAction : public AttackAction
{
public:
    MagtheridonMainTankAttackFirstThreeChannelersAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "magtheridon main tank attack first three channelers") {}
    bool Execute(Event event) override;
};

class MagtheridonFirstAssistTankAttackNWChannelerAction : public AttackAction
{
public:
    MagtheridonFirstAssistTankAttackNWChannelerAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "magtheridon first assist tank attack nw channeler") {}
    bool Execute(Event event) override;
};

class MagtheridonSecondAssistTankAttackNEChannelerAction : public AttackAction
{
public:
    MagtheridonSecondAssistTankAttackNEChannelerAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "magtheridon second assist tank attack ne channeler") {}
    bool Execute(Event event) override;
};

class MagtheridonMisdirectHellfireChannelersToMainTankAction : public AttackAction
{
public:
    MagtheridonMisdirectHellfireChannelersToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "magtheridon misdirect hellfire channelers to main tank") {}
    bool Execute(Event event) override;
};

class MagtheridonAssignDpsPriorityAction : public AttackAction
{
public:
    MagtheridonAssignDpsPriorityAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "magtheridon assign dps priority") {}
    bool Execute(Event event) override;
};

class MagtheridonWarlockCcBurningAbyssalAction : public AttackAction
{
public:
    MagtheridonWarlockCcBurningAbyssalAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "magtheridon warlock cc burning abyssal") {}
    bool Execute(Event event) override;
};

class MagtheridonMainTankPositionBossAction : public AttackAction
{
public:
    MagtheridonMainTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "magtheridon main tank position boss") {}
    bool Execute(Event event) override;
};

class MagtheridonSpreadRangedAction : public MovementAction
{
public:
    MagtheridonSpreadRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "magtheridon spread ranged") {}
    bool Execute(Event event) override;
};

class MagtheridonMoveOutOfDebrisAction : public MovementAction
{
public:
    MagtheridonMoveOutOfDebrisAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "magtheridon move out of debris") {}
    bool Execute(Event event) override;

private:
    bool FindSafePosition(Position& outPos);
};

class MagtheridonUseManticronCubeAction : public MovementAction
{
public:
    MagtheridonUseManticronCubeAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "magtheridon use manticron cube") {}
    bool Execute(Event event) override;

private:
    MagtheridonHelpers::CubeInfo const* GetAssignedCube();
    bool HandleCubeRelease(Unit* magtheridon);
    bool HandleWaitingPhase(const MagtheridonHelpers::CubeInfo& cubeInfo);
    bool HandleCubeInteraction(const MagtheridonHelpers::CubeInfo& cubeInfo, GameObject* cube);
    bool FindSafePositionNearCube(const MagtheridonHelpers::CubeInfo& cubeInfo, float preferredDistance, Position& outPos);
};

class MagtheridonManageTimersAndAssignmentsAction : public Action
{
public:
    MagtheridonManageTimersAndAssignmentsAction(
        PlayerbotAI* botAI) : Action(botAI, "magtheridon manage timers and assignments") {}
    bool Execute(Event event) override;

private:
    bool AssignCubeClickers();
    bool NeedsCubeReassignment(uint32 instanceId);
};

class MagtheridonEraseTimersAndTrackersAction : public Action
{
public:
    MagtheridonEraseTimersAndTrackersAction(
        PlayerbotAI* botAI, std::string const name = "magtheridon erase timers and trackers") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
