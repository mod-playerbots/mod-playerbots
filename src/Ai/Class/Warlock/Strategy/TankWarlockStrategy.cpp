/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TankWarlockStrategy.h"

TankWarlockStrategy::TankWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
    // No custom ActionNodeFactory needed
}

std::vector<NextAction> TankWarlockStrategy::getDefaultActions()
{
    return {
        NextAction("shadow ward", 27.5f),
        NextAction("searing pain", 27.0f)
    };
}

void TankWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& /*triggers*/)
{
}
