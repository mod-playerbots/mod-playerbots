/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GREETACTION_H
#define PLAYERBOTS_GREETACTION_H

#include "Action.h"

class PlayerbotAI;

class GreetAction : public Action
{
public:
    GreetAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
};

#endif
