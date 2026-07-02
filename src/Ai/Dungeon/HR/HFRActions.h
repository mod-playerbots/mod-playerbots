/*
* This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
* information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License, or (at your option) any later version.
*/

#ifndef PLAYERBOTS_HFRACTIONS_H
#define PLAYERBOTS_HFRACTIONS_H

#include "AttackAction.h"
#include "MovementActions.h"
#include "Action.h"

// Watchkeeper Gargolmar

class GargolmarMarkHellfireWatchersAction : public Action
{
public:
    GargolmarMarkHellfireWatchersAction(
        PlayerbotAI* botAI, std::string const name = "gargolmar mark hellfire watchers") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Omor the Unscarred

class OmorTreacheryAuraFleeFromPlayersAction : public MovementAction
{
public:
    OmorTreacheryAuraFleeFromPlayersAction(
        PlayerbotAI* botAI, std::string const name = "omor treachery aura flee from players") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class OmorRangedSpreadAction : public MovementAction
{
public:
    OmorRangedSpreadAction(
        PlayerbotAI* botAI, std::string const name = "omor ranged spread") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class OmorMarkFiendishHoundAction : public Action
{
public:
    OmorMarkFiendishHoundAction(
        PlayerbotAI* botAI, std::string const name = "omor mark fiendish hound") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class OmorTreacheryAuraFleeFromTankAction : public MovementAction
{
public:
    OmorTreacheryAuraFleeFromTankAction(
        PlayerbotAI* botAI, std::string const name = "omor treachery aura flee from tank") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

// Vazruden

class VazrudenTankPositionBossAction : public AttackAction
{
public:
    VazrudenTankPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "vazruden tank position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class VazrudenMarkBossAction : public Action
{
public:
    VazrudenMarkBossAction(
        PlayerbotAI* botAI, std::string const name = "vazruden mark boss") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
