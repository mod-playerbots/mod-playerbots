/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "VHStrategy.h"
#include "VHMultipliers.h"

void WotlkDungeonVHStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Erekem
    triggers.push_back(new TriggerNode("erekem target",
        { NextAction("attack erekem", ACTION_RAID + 1) }));

    // Ichoron
    triggers.push_back(new TriggerNode("ichoron target",
        { NextAction("attack ichor globule", ACTION_RAID + 1) }));

    // Zuramat the Obliterator
    triggers.push_back(new TriggerNode("shroud of darkness",
        { NextAction("drop target", ACTION_HIGH + 5) }));
    triggers.push_back(new TriggerNode("void shift",
        { NextAction("attack void sentry", ACTION_RAID + 1) }));

    // Cyanigosa
    triggers.push_back(new TriggerNode("cyanigosa positioning",
        { NextAction("rear flank", ACTION_MOVE + 5) }));
}

void WotlkDungeonVHStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new ErekemMultiplier(botAI));
    multipliers.push_back(new IchoronMultiplier(botAI));
    multipliers.push_back(new ZuramatMultiplier(botAI));
}
