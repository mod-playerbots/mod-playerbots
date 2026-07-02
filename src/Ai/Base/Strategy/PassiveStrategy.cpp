/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PassiveStrategy.h"

#include "PassiveMultiplier.h"
#include "Playerbots.h"

void PassiveStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new PassiveMultiplier(botAI));
}
