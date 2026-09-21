/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FLAGACTION_H
#define PLAYERBOTS_FLAGACTION_H

#include "Action.h"

class PlayerbotAI;

class FlagAction : public Action
{
public:
    FlagAction(PlayerbotAI* botAI) : Action(botAI, "flag") {}

    bool Execute(Event event) override;

private:
    bool TellUsage();
};

#endif
