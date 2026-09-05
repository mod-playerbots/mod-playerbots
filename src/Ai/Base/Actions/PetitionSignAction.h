/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PETITIONSIGNACTION_H
#define PLAYERBOTS_PETITIONSIGNACTION_H

#include "Action.h"

class PlayerbotAI;

class PetitionSignAction : public Action
{
public:
    PetitionSignAction(PlayerbotAI* botAI) : Action(botAI, "petition sign") {}

    bool Execute(Event event) override;
};

#endif
