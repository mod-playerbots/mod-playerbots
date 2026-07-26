/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UBMultipliers.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "UBActions.h"
#include "UBShared.h"

using namespace UnderbogHungarfen;

float HungarfenFoulSporesMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "hungarfen");
    if (!boss || !boss->HasAura(SPELL_FOUL_SPORES))
        return 1.0f;

    if (dynamic_cast<UBRetreatFromFoulSporesAction*>(action) || dynamic_cast<UBVacateSporeCloudAction*>(action) ||
        dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<MovementAction*>(action) || dynamic_cast<CastReachTargetSpellAction*>(action))
        return 0.0f;

    return 1.0f;
}

float HungarfenMushroomIgnoreMultiplier::GetValue(Action* action)
{
    if (action->getThreatType() != Action::ActionThreatType::Aoe)
        return 1.0f;

    if (!botAI->IsDps(bot))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "hungarfen"))
        return 1.0f;

    return 0.0f;
}
