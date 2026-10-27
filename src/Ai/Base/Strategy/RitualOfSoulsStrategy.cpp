/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RitualOfSoulsStrategy.h"

#include "Playerbots.h"

void RitualOfSoulsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("ritual of souls portal nearby", { NextAction("join ritual of souls", 27.0f) }));
    triggers.push_back(new TriggerNode("soulwell nearby", { NextAction("use soulwell", 26.0f) }));
}
