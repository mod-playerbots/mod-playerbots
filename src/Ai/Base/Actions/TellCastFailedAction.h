/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TELLCASTFAILEDACTION_H
#define PLAYERBOTS_TELLCASTFAILEDACTION_H

#include "Action.h"

class PlayerbotAI;

class TellSpellAction : public Action
{
public:
    TellSpellAction(PlayerbotAI* botAI) : Action(botAI, "spell") {}

    bool Execute(Event event) override;
};

class TellCastFailedAction : public Action
{
public:
    TellCastFailedAction(PlayerbotAI* botAI) : Action(botAI, "tell cast failed") {}

    bool Execute(Event event) override;
};

#endif
