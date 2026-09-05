/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HIREACTION_H
#define PLAYERBOTS_HIREACTION_H

#include "Action.h"

class PlayerbotAI;

class HireAction : public Action
{
public:
    HireAction(PlayerbotAI* botAI) : Action(botAI, "hire") {}

    bool Execute(Event event) override;
};

#endif
