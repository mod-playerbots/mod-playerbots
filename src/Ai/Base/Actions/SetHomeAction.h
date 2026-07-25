/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SETHOMEACTION_H
#define PLAYERBOTS_SETHOMEACTION_H

#include "MovementActions.h"

class PlayerbotAI;

class SetHomeAction : public MovementAction
{
public:
    SetHomeAction(PlayerbotAI* botAI) : MovementAction(botAI, "home") {}

    bool Execute(Event event) override;
};

#endif
