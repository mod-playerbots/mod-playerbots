/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "OnyMultipliers.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "OnyHelpers.h"
#include "Playerbots.h"

using namespace OnyxiaHelpers;

float OnyxiaPhaseThreeThreatMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<AttackAction*>(action) &&
        (!dynamic_cast<CastSpellAction*>(action) || dynamic_cast<CastHealingSpellAction*>(action)))
    {
        return 1.0f;
    }

    Unit* target = action->GetTarget();
    if (!IsOnyxia(target))
        return 1.0f;

    return MustHoldPhaseThreeThreat(bot, target, THREAT_LEAD) ? 0.0f : 1.0f;
}
