#ifndef _PLAYERBOT_RAIDMAGTHERIDONACTIONS_H
#define _PLAYERBOT_RAIDMAGTHERIDONACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

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

class MagtheridonHellfireChannelerDPSPriorityAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerDPSPriorityAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler dps priority") : AttackAction(botAI, name) {};

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
};

class MagtheridonResetTimersAndAssignmentsAction : public Action
{
public:
    MagtheridonResetTimersAndAssignmentsAction(PlayerbotAI* botAI, std::string const name = "magtheridon reset timers and assignments") : Action(botAI, name) {};

    bool Execute(Event event) override;
};

#endif
