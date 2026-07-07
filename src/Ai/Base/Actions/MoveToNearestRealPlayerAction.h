/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_MOVETONEARESTREALPLAYERACTION_H
#define PLAYERBOTS_MOVETONEARESTREALPLAYERACTION_H

#include "MovementActions.h"

class PlayerbotAI;

// Walks a world-PvP bot toward the nearest real player in its assigned zone, re-deriving the
// destination every tick so it tracks a moving target across the long distance left by the
// bot's initial teleport (AiPlayerbot.SyncBotsWithPlayerMinDistance/MaxDistance).
class MoveToNearestRealPlayerAction : public MovementAction
{
public:
    MoveToNearestRealPlayerAction(PlayerbotAI* botAI) : MovementAction(botAI, "move to nearest real player") {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    Player* GetPursueTarget();
};

#endif
