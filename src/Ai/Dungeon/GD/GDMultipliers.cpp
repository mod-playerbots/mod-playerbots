/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GDMultipliers.h"
#include "Action.h"
#include "ChooseTargetActions.h"
#include "GDActions.h"
#include "GDTriggers.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"

float SladranMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "slad'ran");
    if (!boss) { return 1.0f; }

    if (boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA))
    {
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AvoidPoisonNovaAction*>(action))
        {
            return 0.0f;
        }
    }

    if (!botAI->IsDps(bot)) { return 1.0f; }

    if (action->getThreatType() == Action::ActionThreatType::Aoe)
    {
        return 0.0f;
    }

    Unit* snakeWrap = nullptr;
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->GetEntry() == NPC_SNAKE_WRAP)
        {
            snakeWrap = unit;
            break;
        }
    }
    // Prevent auto-target acquisition during snake wraps
    if (snakeWrap && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float GaldarahMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "gal'darah");
    if (!boss) { return 1.0f; }

    if (boss->HasAura(SPELL_WHIRLING_SLASH))
        {
            if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AvoidWhirlingSlashAction*>(action))
            {
                return 0.0f;
            }
        }
    return 1.0f;
}
