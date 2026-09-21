/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_WORLDBUFFACTION_H
#define PLAYERBOTS_WORLDBUFFACTION_H

#include "Action.h"

class PlayerbotAI;
class Unit;

class WorldBuffAction : public Action
{
public:
    WorldBuffAction(PlayerbotAI* botAI) : Action(botAI, "world buff") {}

    bool Execute(Event event) override;

    static std::vector<uint32> NeedWorldBuffs(Unit* unit);
};

#endif
