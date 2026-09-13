/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RampMultipliers.h"
#include "EncounterHelpers.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "Playerbots.h"
#include "RampActions.h"
#include "RampTriggers.h"
#include "ReachTargetActions.h"

using namespace EncounterHelpers;

// Omor the Unscarred

float OmorTreacheryAuraFleeFromPlayersMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<OmorRangedSpreadAction*>(action) ||
        dynamic_cast<OmorTreacheryAuraFleeFromPlayersAction*>(action) ||
        dynamic_cast<OmorTreacheryAuraFleeFromTankAction*>(action) ||
        dynamic_cast<OmorMarkFiendishHoundAction*>(action) ||
        dynamic_cast<ReachPartyMemberToHealAction*>(action))
        return 1.0f;

    bool const isMovementSpell = dynamic_cast<CastReachTargetSpellAction*>(action) ||
                                 dynamic_cast<CastBlinkBackAction*>(action) ||
                                 dynamic_cast<CastDisengageAction*>(action);

    if (!isMovementSpell && !dynamic_cast<MovementAction*>(action))
        return 1.0f;

    Unit* omor = AI_VALUE2(Unit*, "find target", "omor the unscarred");

    if (!omor)
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    if (PlayerbotAI::IsRanged(bot) && dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (PlayerbotAI::IsMelee(bot) &&
        bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_BANE_OF_TREACHERY)) ||
        bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_TREACHEROUS_AURA)))
        return 0.0f;

    Player* tank = GetGroupMainTank(bot);

    if (tank &&
        (tank->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_BANE_OF_TREACHERY)) ||
         tank->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_TREACHEROUS_AURA))))
        return 0.0f;

     return 1.0f;
}

// Vazruden

float VazrudenDisableTankAssistMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    Unit* vazruden = AI_VALUE2(Unit*, "find target", "vazruden");

    if (!vazruden)
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
