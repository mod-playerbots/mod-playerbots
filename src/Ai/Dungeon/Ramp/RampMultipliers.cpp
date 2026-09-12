/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RampMultipliers.h"
#include "ChooseTargetActions.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "RampActions.h"
#include "RampTriggers.h"
#include "ReachTargetActions.h"

using namespace EncounterHelpers;

// Omor the Unscarred

float OmorTreacheryAuraFleeFromPlayersMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_BANE_OF_TREACHERY)) &&
        !bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_TREACHEROUS_AURA)))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Vazruden

float VazrudenDisableTankAssistMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "vazruden"))
        return 1.0f;

    Unit* nazan = AI_VALUE2(Unit*, "find target", "nazan");
    if (!nazan)
        return 1.0f;

    if (!nazan->IsFlying())
        return 1.0f;  

    Unit* target = action->GetTarget();
    if (target->GetName() == "nazan")
        return 0.0f;

    return 1.0f;   
}
