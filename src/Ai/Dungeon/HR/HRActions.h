#ifndef _PLAYERBOT_TBCDUNGEONHELLFIRERAMPARTSACTIONS_H
#define _PLAYERBOT_TBCDUNGEONHELLFIRERAMPARTSACTIONS_H

#include "AttackAction.h"
#include "MovementActions.h"
#include "Action.h"
#include "HRTriggers.h"

// Watchkeeper Gargolmar

class GargolmarTankPositionBossAction : public AttackAction
{
public:
    GargolmarTankPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "gargolmar tank position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class GargolmarMarkHellfireWatchersAction : public Action
{
public:
    GargolmarMarkHellfireWatchersAction(
        PlayerbotAI* botAI, std::string const name = "gargolmar mark hellfire watchers") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Omor the Unscarred

class OmorTreacherousAuraFleeFromPlayersAction : public Action
{
public:
    OmorTreacherousAuraFleeFromPlayersAction(
        PlayerbotAI* botAI, std::string const name = "omor treacherous aura flee from players") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class OmorBaneOfTreacheryAuraFleeFromPlayersAction : public Action
{
public:
    OmorBaneOfTreacheryAuraFleeFromPlayersAction(
        PlayerbotAI* botAI, std::string const name = "omor bane of treachery aura flee from players") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class OmorRangedSpreadAction : public MovementAction
{
public:
    OmorRangedSpreadAction(
        PlayerbotAI* botAI, std::string const name = "omor ranged spread") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
