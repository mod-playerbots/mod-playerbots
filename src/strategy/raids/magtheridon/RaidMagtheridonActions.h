#ifndef _PLAYERBOT_RAIDMAGTHERIDONACTIONS_H
#define _PLAYERBOT_RAIDMAGTHERIDONACTIONS_H

#include "RaidMagtheridonHelpers.h"
#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

using namespace MagtheridonHelpers;

class MagtheridonMainTankAttackFirstThreeChannelersAction : public AttackAction
{
public:
    MagtheridonMainTankAttackFirstThreeChannelersAction(PlayerbotAI* botAI, std::string const name = "magtheridon main tank attack first three channelers") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class MagtheridonFirstAssistTankAttackNWChannelerAction : public AttackAction
{
public:
    MagtheridonFirstAssistTankAttackNWChannelerAction(PlayerbotAI* botAI, std::string const name = "magtheridon first assist tank attack nw channeler") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class MagtheridonSecondAssistTankAttackNEChannelerAction : public AttackAction
{
public:
    MagtheridonSecondAssistTankAttackNEChannelerAction(PlayerbotAI* botAI, std::string const name = "magtheridon second assist tank attack ne channeler") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class MagtheridonMisdirectHellfireChannelers : public AttackAction
{
public:
    MagtheridonMisdirectHellfireChannelers(PlayerbotAI* botAI, std::string const name = "magtheridon misdirect hellfire channelers") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class MagtheridonAssignDPSPriorityAction : public AttackAction
{
public:
    MagtheridonAssignDPSPriorityAction(PlayerbotAI* botAI, std::string const name = "magtheridon assign dps priority") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class MagtheridonWarlockCCBurningAbyssalAction : public AttackAction
{
public:
    MagtheridonWarlockCCBurningAbyssalAction(PlayerbotAI* botAI, std::string const name = "magtheridon warlock cc burning abyssal") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class MagtheridonMainTankPositionBossAction : public AttackAction
{
public:
    MagtheridonMainTankPositionBossAction(PlayerbotAI* botAI, std::string const name = "magtheridon main tank position boss") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class MagtheridonSpreadRangedAction : public MovementAction
{
public:
    static std::unordered_map<ObjectGuid, Position> initialPositions;
    static std::unordered_map<ObjectGuid, bool> hasReachedInitialPosition;

    MagtheridonSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "magtheridon spread ranged") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
};

class MagtheridonUseManticronCubeAction : public MovementAction
{
public:
    MagtheridonUseManticronCubeAction(PlayerbotAI* botAI, std::string const name = "magtheridon use manticron cube") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;

private:
    bool HandleCubeRelease(Unit* magtheridon, GameObject* cube);
    bool ShouldActivateCubeLogic(Unit* magtheridon);
    bool HandleWaitingPhase(const CubeInfo& cubeInfo);
    bool HandleCubeInteraction(const CubeInfo& cubeInfo, GameObject* cube);
};

class MagtheridonManageTimersAndAssignmentsAction : public Action
{
public:
    MagtheridonManageTimersAndAssignmentsAction(PlayerbotAI* botAI, std::string const name = "magtheridon manage timers and assignments") : Action(botAI, name) {};

    bool Execute(Event event) override;
};

#endif
