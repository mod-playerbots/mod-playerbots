/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ANStrategy.h"
#include "ANMultipliers.h"

void WotlkDungeonANStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Krik'thir the Gatewatcher
    triggers.push_back(new TriggerNode("krik'thir web wrap",
        { NextAction("attack web wrap", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("krik'thir watchers",
        { NextAction("krik'thir priority", ACTION_RAID + 4) }));

    //Anub'arak
    triggers.push_back(new TriggerNode("anub'arak pound",
        { NextAction("dodge pound", ACTION_MOVE + 5) }));
}

void WotlkDungeonANStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new KrikthirMultiplier(botAI));
}
