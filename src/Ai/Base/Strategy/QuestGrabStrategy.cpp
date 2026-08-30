/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "QuestGrabStrategy.h"

void QuestGrabStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Same periodic "timer" trigger GatherStrategy already uses for
    // "add gathering loot" (LootNonCombatStrategy.cpp). Relevance matches
    // where "new rpg status update" sits today, well above plain "follow"'s
    // baseline (1.0), so this wins whenever something's actually grabbable
    // and yields back to "follow" the instant GrabQuestItemAction::Execute
    // returns false.
    triggers.push_back(new TriggerNode("timer", { NextAction("grab quest item", 11.0f) }));
}
