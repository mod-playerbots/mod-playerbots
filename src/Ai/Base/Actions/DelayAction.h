/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DELAYACTION_H
#define PLAYERBOTS_DELAYACTION_H

#include "Action.h"

class PlayerbotAI;

class DelayAction : public Action
{
public:
    DelayAction(PlayerbotAI* botAI) : Action(botAI, "delay") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
