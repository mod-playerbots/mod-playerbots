/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MOVETORPGTARGETACTION_H
#define PLAYERBOTS_MOVETORPGTARGETACTION_H

#include "MovementActions.h"

class PlayerbotAI;

class MoveToRpgTargetAction : public MovementAction
{
public:
    MoveToRpgTargetAction(PlayerbotAI* botAI) : MovementAction(botAI, "move to rpg target") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
