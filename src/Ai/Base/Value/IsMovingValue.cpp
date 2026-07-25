/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "IsMovingValue.h"

#include "Playerbots.h"

bool IsMovingValue::Calculate()
{
    Unit* target = AI_VALUE(Unit*, qualifier);

    if (!target)
        return false;

    return target->isMoving();
}

bool IsSwimmingValue::Calculate()
{
    Unit* target = AI_VALUE(Unit*, qualifier);

    if (!target)
        return false;

    int8 targetInLiquidState = target->GetLiquidData().Status;

    return targetInLiquidState == LIQUID_MAP_UNDER_WATER || (targetInLiquidState == LIQUID_MAP_IN_WATER && target->CanSwim());
}
