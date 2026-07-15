/*
* This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
* information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License, or (at your option) any later version.
*/

#include "HFRMultipliers.h"
#include "HFRActions.h"
#include "HFRTriggers.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "ReachTargetActions.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

// Omor the Unscarred

float OmorTreacheryAuraFleeFromPlayersMultiplier::GetValue(Action* action)
{
    Unit* omor = AI_VALUE2(Unit*, "find target", "omor the unscarred");
    if (!omor)
        return 1.0f;

    if (!bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_BANE_OF_TREACHERY)) &&
        !bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_TREACHEROUS_AURA)))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        (dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<OmorTreacheryAuraFleeFromPlayersAction*>(action)))
        return 0.0f;

    return 1.0f;
}

float OmorTreacheryAuraFleeFromTankMultiplier::GetValue(Action* action)
{
    Unit* omor = AI_VALUE2(Unit*, "find target", "omor the unscarred");
    if (!omor)
        return 1.0f;

    Player* tank = GetGroupMainTank(botAI, bot);
    if (!tank)
        return 1.0f;

    if (!tank->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_BANE_OF_TREACHERY)) &&
        !tank->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_TREACHEROUS_AURA)))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        (dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<OmorTreacheryAuraFleeFromTankAction*>(action)))
        return 0.0f;

    return 1.0f;
}

// Vazruden

float VazrudenDisableTankAssistMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot) ||
        !AI_VALUE2(Unit*, "find target", "vazruden"))
        return 1.0f;

    if (Unit* nazan = AI_VALUE2(Unit*, "find target", "nazan"))
    {
        if (Creature* nazanCreature = nazan->ToCreature())
        {
            if (!nazanCreature->CanFly())
                return 1.0f;
        }
    }

    if (bot->GetVictim() != nullptr &&
        dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}
