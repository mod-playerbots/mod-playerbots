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
        PlayerbotAI* botAI, std::string const name = "magtheridon main tank attack first three channelers") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class MagtheridonFirstAssistTankAttackNWChannelerAction : public AttackAction
{
public:
    MagtheridonFirstAssistTankAttackNWChannelerAction(
        PlayerbotAI* botAI, std::string const name = "magtheridon first assist tank attack nw channeler") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class MagtheridonSecondAssistTankAttackNEChannelerAction : public AttackAction
{
public:
    MagtheridonSecondAssistTankAttackNEChannelerAction(
        PlayerbotAI* botAI, std::string const name = "magtheridon second assist tank attack ne channeler") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class MagtheridonMisdirectHellfireChannelersToMainTankAction : public AttackAction
{
public:
    MagtheridonMisdirectHellfireChannelersToMainTankAction(
        PlayerbotAI* botAI, std::string const name = "magtheridon misdirect hellfire channelers to main tank") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class MagtheridonAssignDpsPriorityAction : public AttackAction
{
public:
    MagtheridonAssignDpsPriorityAction(
        PlayerbotAI* botAI, std::string const name = "magtheridon assign dps priority") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class MagtheridonWarlockCcBurningAbyssalAction : public AttackAction
{
public:
    MagtheridonWarlockCcBurningAbyssalAction(
        PlayerbotAI* botAI, std::string const name = "magtheridon warlock cc burning abyssal") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class MagtheridonMainTankPositionBossAction : public AttackAction
{
public:
    MagtheridonMainTankPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "magtheridon main tank position boss") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class MagtheridonSpreadRangedAction : public MovementAction
{
public:
    MagtheridonSpreadRangedAction(
        PlayerbotAI* botAI, std::string const name = "magtheridon spread ranged") : MovementAction(botAI, name) {};
    bool Execute(Event event) override;
};

class MagtheridonMoveOutOfDebrisAction : public MovementAction
{
public:
    MagtheridonMoveOutOfDebrisAction(
        PlayerbotAI* botAI, std::string const name = "magtheridon move out of debris") : MovementAction(botAI, name) {};
    bool Execute(Event event) override;

private:
    bool FindSafePosition(Position& outPos);
};

class MagtheridonUseManticronCubeAction : public MovementAction
{
public:
    MagtheridonUseManticronCubeAction(
        PlayerbotAI* botAI, std::string const name = "magtheridon use manticron cube") : MovementAction(botAI, name) {};
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
        PlayerbotAI* botAI, std::string const name = "magtheridon manage timers and assignments") : Action(botAI, name) {};
    bool Execute(Event event) override;

private:
    bool AssignCubeClickers();
    bool NeedsCubeReassignment(uint32 instanceId);
};

#endif
