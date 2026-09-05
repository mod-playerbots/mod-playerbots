/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TELLTARGETACTION_H
#define PLAYERBOTS_TELLTARGETACTION_H

#include "Action.h"

class PlayerbotAI;

class TellTargetAction : public Action
{
public:
    TellTargetAction(PlayerbotAI* botAI) : Action(botAI, "tell target") {}

    bool Execute(Event event) override;
};

class TellAttackersAction : public Action
{
public:
    TellAttackersAction(PlayerbotAI* botAI) : Action(botAI, "tell attackers") {}

    bool Execute(Event event) override;
};

#endif
