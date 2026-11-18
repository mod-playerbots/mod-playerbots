/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCTriggers_Staging.h"

#include "Playerbots.h"
#include "ToCHelpers_Staging.h"

using namespace TrialOfTheCrusaderHelpers;

bool ToCStagingStandTrigger::IsActive()
{
    ToCStagingRead const read = GetToCStagingRead(botAI);
    return read.held && read.walk;
}
