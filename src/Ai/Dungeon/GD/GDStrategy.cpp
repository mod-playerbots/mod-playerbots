/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GDStrategy.h"
#include "GDMultipliers.h"

void WotlkDungeonGDStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Slad'ran
    triggers.push_back(new TriggerNode("poison nova",
        { NextAction("avoid poison nova", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("snake wrap",
        { NextAction("attack snake wrap", ACTION_RAID + 4) }));
    triggers.push_back(new TriggerNode("slad'ran stack on tank",
        { NextAction("slad'ran stack on tank", ACTION_RAID + 1) }));
    triggers.push_back(new TriggerNode("slad'ran tank hold",
        { NextAction("slad'ran tank hold", ACTION_RAID + 1) }));

    // Gal'darah
    triggers.push_back(new TriggerNode("whirling slash",
        { NextAction("avoid whirling slash", ACTION_RAID + 5) }));
}

void WotlkDungeonGDStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new SladranMultiplier(botAI));
    multipliers.push_back(new GaldarahMultiplier(botAI));
}
