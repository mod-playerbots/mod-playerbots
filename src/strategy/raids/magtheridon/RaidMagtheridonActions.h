#ifndef _PLAYERBOT_RAIDMAGTHERIDONACTIONS_H
#define _PLAYERBOT_RAIDMAGTHERIDONACTIONS_H

#include "RaidMagtheridonHelpers.h"
#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

using namespace MagtheridonHelpers;

class MagtheridonHellfireChannelerMainTankAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerMainTankAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler main tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class MagtheridonHellfireChannelerNWChannelerTankAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerNWChannelerTankAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler nw channeler tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class MagtheridonHellfireChannelerNEChannelerTankAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerNEChannelerTankAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler ne channeler tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class MagtheridonHellfireChannelerMisdirectionAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerMisdirectionAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler misdirection") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class MagtheridonDPSPriorityAction : public AttackAction
{
public:
    MagtheridonDPSPriorityAction(PlayerbotAI* botAI, std::string const name = "magtheridon dps priority") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class MagtheridonBurningAbyssalWarlockCCAction : public AttackAction
{
public:
    MagtheridonBurningAbyssalWarlockCCAction(PlayerbotAI* botAI, std::string const name = "magtheridon burning abyssal warlock cc") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class MagtheridonPositionBossAction : public AttackAction
{
public:
    MagtheridonPositionBossAction(PlayerbotAI* botAI, std::string const name = "magtheridon position boss") : AttackAction(botAI, name) {};

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
