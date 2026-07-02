/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RANDOMBOTUPDATEACTION_H
#define PLAYERBOTS_RANDOMBOTUPDATEACTION_H

#include "Action.h"

class PlayerbotAI;

class RandomBotUpdateAction : public Action
{
public:
    RandomBotUpdateAction(PlayerbotAI* botAI) : Action(botAI, "random bot update") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
