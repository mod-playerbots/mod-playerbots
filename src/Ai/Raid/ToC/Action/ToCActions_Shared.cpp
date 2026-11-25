/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCActions_Shared.h"
#include "ToCData.h"
#include "Playerbots.h"

#include <algorithm>

using namespace TrialOfTheCrusaderHelpers;

bool ToCMainTankHoldAction::DragBossToAnchor(Unit* boss, Position const& anchor)
{
    if (boss->GetVictim() != bot)
        return false;

    const float distToPosition = bot->GetExactDist2d(anchor.GetPositionX(), anchor.GetPositionY());
    if (distToPosition <= TOC_DRAG_DONE)
        return false;

    const float dX = anchor.GetPositionX() - bot->GetPositionX();
    const float dY = anchor.GetPositionY() - bot->GetPositionY();
    const float moveDist = std::min(TOC_DRAG_STEP, distToPosition);
    const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
    const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

    return MoveTo(TRIAL_OF_THE_CRUSADER_MAP_ID, moveX, moveY, anchor.GetPositionZ(),
                  false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
}
