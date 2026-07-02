/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GOACTION_H
#define PLAYERBOTS_GOACTION_H

#include "MovementActions.h"

class PlayerbotAI;
class TravelDestination;
class WorldPosition;

class GoAction : public MovementAction
{
public:
    GoAction(PlayerbotAI* botAI) : MovementAction(botAI, "Go") {}

    bool Execute(Event event) override;
};

#endif
