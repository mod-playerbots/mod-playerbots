/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_WORLDPVPPURSUESTRATEGY_H
#define PLAYERBOTS_WORLDPVPPURSUESTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

// Toggled on/off by RandomPlayerbotMgr for bots it has marked/released as world-PvP bots. Gives the
// bot a goal to close the distance to the nearest real player in its zone; yields to normal combat
// as soon as an opposing-faction unit is nearby (see WorldPvpPursueTrigger/MoveToNearestRealPlayerAction).
class WorldPvpPursueStrategy : public NonCombatStrategy
{
public:
    WorldPvpPursueStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    std::string const getName() override { return "world pvp pursue"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
