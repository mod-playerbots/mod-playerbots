/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCActions_NorthrendBeasts.h"

#include "ToCHelpers_NorthrendBeasts.h"

using namespace TrialOfTheCrusaderHelpers;

bool NorthrendBeastsDpsTargetAction::Execute(Event /*event*/)
{
    return AttackDirect(GetBeastsDpsTarget(botAI));
}
