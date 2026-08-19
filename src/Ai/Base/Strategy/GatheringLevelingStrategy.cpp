/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GatheringLevelingStrategy.h"

#include "Playerbots.h"

GatheringLevelingStrategy::GatheringLevelingStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

std::vector<NextAction> GatheringLevelingStrategy::getDefaultActions()
{
    // Steady low-priority heartbeat: runs the state machine when nothing more
    // important (looting/harvesting) is happening.
    return {NextAction("gather leveling update", 3.5f)};
}

void GatheringLevelingStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Seeing a node beyond our skill is a strong signal to prioritise going to
    // a trainer / starting to level the profession.
    triggers.push_back(
        new TriggerNode("need gather leveling", {NextAction("gather leveling update", 40.0f)}));
}
