/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CoSMultipliers.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "ThreatHelpers.h"

float EpochMultiplier::GetValue(Action* action)
{
    if (!FindThreatTarget(bot, "chrono-lord epoch")) { return 1.0f; }

    if (bot->getClass() == CLASS_HUNTER) { return 1.0f; }

    if (dynamic_cast<FleeAction*>(action)) { return 0.0f; }

    return 1.0f;
}
