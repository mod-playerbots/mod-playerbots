/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CHANGESTRATEGYACTION_H
#define PLAYERBOTS_CHANGESTRATEGYACTION_H

#include "Action.h"

class PlayerbotAI;

class ChangeCombatStrategyAction : public Action
{
public:
    ChangeCombatStrategyAction(PlayerbotAI* botAI, std::string const name = "co") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class ChangeNonCombatStrategyAction : public Action
{
public:
    ChangeNonCombatStrategyAction(PlayerbotAI* botAI) : Action(botAI, "nc") {}

    bool Execute(Event event) override;
};

class ChangeDeadStrategyAction : public Action
{
public:
    ChangeDeadStrategyAction(PlayerbotAI* botAI) : Action(botAI, "de") {}

    bool Execute(Event event) override;
};

#endif
