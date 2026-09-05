/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FishingTriggers.h"
#include "Playerbots.h"

bool CanFishTrigger::IsActive() { return AI_VALUE(bool, "can fish"); }

bool CanUseFishingBobberTrigger::IsActive() { return AI_VALUE(bool, "can use fishing bobber");}
