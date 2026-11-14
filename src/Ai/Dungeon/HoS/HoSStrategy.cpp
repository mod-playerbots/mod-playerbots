/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HoSStrategy.h"
#include "HoSMultipliers.h"

void WotlkDungeonHoSStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Krystallus
    triggers.push_back(new TriggerNode("ground slam",
        { NextAction("shatter spread", ACTION_RAID + 5) }));

    // Sjonnir The Ironshaper
    triggers.push_back(new TriggerNode("lightning ring",
        { NextAction("avoid lightning ring", ACTION_RAID + 5) }));
}

void WotlkDungeonHoSStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new KrystallusMultiplier(botAI));
    multipliers.push_back(new SjonnirMultiplier(botAI));
}
