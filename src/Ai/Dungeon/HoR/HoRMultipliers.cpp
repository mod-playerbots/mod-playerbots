/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HoRMultipliers.h"
#include "ChooseTargetActions.h"
#include "FollowActions.h"
#include "HoRActions.h"
#include "Playerbots.h"

float HorEscapeMultiplier::GetValue(Action* action)
{
    if (!HorEscapeActive(botAI))
        return 1.0f;

    // Our own escape actions and following the leader are always allowed.
    if (dynamic_cast<HorEscapeAttackAction*>(action) || dynamic_cast<HorEscapeTankAction*>(action) ||
        dynamic_cast<HorEscapeFollowAction*>(action) || dynamic_cast<FollowAction*>(action))
        return 1.0f;

    // Take over the standard target-choosing pipeline while the chase runs.
    if (dynamic_cast<TankAssistAction*>(action) || dynamic_cast<AggressiveTargetAction*>(action) ||
        dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<DpsAoeAction*>(action))
        return 0.0f;

    // Never attack the Lich King - the event is a chase, not a kill.
    if (AttackAction* attack = dynamic_cast<AttackAction*>(action))
    {
        Unit* target = attack->GetTarget();
        if (target && target->GetEntry() == NPC_HOR_LICH_KING)
            return 0.0f;
    }

    return 1.0f;
}
