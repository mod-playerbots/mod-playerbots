/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GuildStrategy.h"

#include "Playerbots.h"

void GuildStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("often", { NextAction("offer petition nearby", 4.0f) }));
    triggers.push_back(
        new TriggerNode("often", { NextAction("guild manage nearby", 4.0f) }));
    triggers.push_back(
        new TriggerNode("petition signed", { NextAction("turn in petition", 10.0f) }));
    triggers.push_back(
        new TriggerNode("buy tabard", { NextAction("buy tabard", 10.0f) }));
    triggers.push_back(
        new TriggerNode("leave large guild", { NextAction("guild leave", 4.0f) }));
}
