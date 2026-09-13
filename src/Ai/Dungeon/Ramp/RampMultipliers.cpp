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
    // Allow all Omor fight specific actions
    if (dynamic_cast<OmorRangedSpreadAction*>(action) ||
        dynamic_cast<OmorTreacheryAuraFleeFromPlayersAction*>(action) ||
        dynamic_cast<OmorTreacheryAuraFleeFromTankAction*>(action) ||
        dynamic_cast<OmorMarkFiendishHoundAction*>(action))
        return 1.0f;

    bool const isMovementSpell = dynamic_cast<CastReachTargetSpellAction*>(action) ||
                                 dynamic_cast<CastBlinkBackAction*>(action) ||
                                 dynamic_cast<CastDisengageAction*>(action);

    // Allow non-movement based Actions
    if (!isMovementSpell && !dynamic_cast<MovementAction*>(action))
        return 1.0f;

    Unit* omor = AI_VALUE2(Unit*, "find target", "omor the unscarred");

    // If Omor isn't found, allow all actions
    if (!omor)
        return 1.0f;

    // Let the tank bot do anything
    if (PlayerbotAI::IsMainTank(bot) ||
        PlayerbotAI::IsRanged(bot))
        return 1.0f;

    // Melee bots try to kill themselves more so need more logic
    if (PlayerbotAI::IsMelee(bot))
    {
        // If melee bot has Aura - don't attack
        if (bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_BANE_OF_TREACHERY)) ||
            bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_TREACHEROUS_AURA)))
            return 0.0f;

        // If tank exists and has Aura - don't attack
        Player* tank = GetGroupMainTank(bot);
        if (tank &&
            (tank->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_BANE_OF_TREACHERY)) ||
            tank->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_TREACHEROUS_AURA))))
            return 0.0f;

        // Edge-case to try and prevent wipes if there is no main tank, or Omor is focusing a non-tank - don't attack
        Unit* omorVictim = omor->GetVictim();
        if (omorVictim &&
            (omorVictim->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_BANE_OF_TREACHERY)) ||
            omorVictim->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_TREACHEROUS_AURA))))
            return 0.0f;

        // It should be safe to attack
        return 1.0f;
    }

    return 0.0f;
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
