/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ARENATEAMACTIONS_H
#define PLAYERBOTS_ARENATEAMACTIONS_H

#include "Action.h"

class PlayerbotAI;

class ArenaTeamAcceptAction : public Action
{
public:
    ArenaTeamAcceptAction(PlayerbotAI* botAI) : Action(botAI, "arena team accept") {}

    bool Execute(Event event) override;
};

#endif
