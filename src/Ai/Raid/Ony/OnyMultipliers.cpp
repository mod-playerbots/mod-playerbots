/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "OnyMultipliers.h"
#include "AttackAction.h"
#include "EncounterHelpers.h"
#include "OnyHelpers.h"
#include "Playerbots.h"
#include "WarlockActions.h"

using namespace OnyxiaHelpers;
using namespace EncounterHelpers;

float OnyxiaPhaseThreeThreatMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsHoldCandidate(bot, action))
        return 1.0f;

    // Soulshatter takes threat off a warlock who has pulled her
    if (dynamic_cast<CastSoulshatterAction*>(action))
        return 1.0f;

    // AttackAction keeps healers in their combat engines. A healer in melee range also swings, but the
    // threat is negligible.
    bool const isHealer = PlayerbotAI::IsHeal(bot);
    if (isHealer && dynamic_cast<AttackAction*>(action))
        return 1.0f;

    Unit* target = action->GetTarget();
    if (!IsOnyxia(target))
    {
        // Only shaman totems are held when not aimed at her (Fire Elemental is cast on self). Attacks and
        // casts on whelps go on.
        if (isHealer || bot->getClass() != CLASS_SHAMAN || dynamic_cast<AttackAction*>(action) ||
            (target && bot->IsHostileTo(target)) || GetDpsHoldValue(bot, action) == 1.0f)
        {
            return 1.0f;
        }

        target = FindOnyxia(bot);
        if (!target)
            return 1.0f;
    }

    return MustHoldPhaseThreeThreat(bot, target, THREAT_LEAD) ? 0.0f : 1.0f;
}
