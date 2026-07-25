/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TellMasterAction.h"

#include "Event.h"
#include "Playerbots.h"

bool TellMasterAction::Execute(Event /*event*/)
{
    botAI->TellMaster(text);
    return true;
}

bool OutOfReactRangeAction::Execute(Event /*event*/)
{
    botAI->TellMaster("Wait for me!");
    return true;
}

bool OutOfReactRangeAction::isUseful()
{
    bool canFollow = Follow(AI_VALUE(Unit*, "group leader"));
    if (!canFollow)
    {
        return false;
    }

    return true;
}
