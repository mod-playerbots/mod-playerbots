/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LOOTSTRATEGYACTION_H
#define PLAYERBOTS_LOOTSTRATEGYACTION_H

#include "Action.h"

class PlayerbotAI;

class LootStrategyAction : public Action
{
public:
    LootStrategyAction(PlayerbotAI* botAI) : Action(botAI, "ll") {}

    bool Execute(Event event) override;
};

#endif
