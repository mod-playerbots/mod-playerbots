/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HoRActions.h"
#include "HoRMultipliers.h"
#include "HoRStrategy.h"
#include "HoRTriggers.h"
#include "Playerbots.h"

void WotlkDungeonHoRStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "hor escape phase",
        {
            NextAction("hor escape tank", ACTION_RAID + 4),
            NextAction("hor escape attack", ACTION_RAID + 3),
            NextAction("hor escape follow", ACTION_MOVE + 2),
        }));
}

void WotlkDungeonHoRStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new HorEscapeMultiplier(botAI));
}
