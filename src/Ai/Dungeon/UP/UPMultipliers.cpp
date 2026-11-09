/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UPMultipliers.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "ThreatHelpers.h"
#include "UPActions.h"
#include "UPTriggers.h"

float SkadiMultiplier::GetValue(Action* action)
{
    Unit* boss = FindThreatTarget(bot, "skadi the ruthless");
    if (!boss) { return 1.0f; }

    Unit* bossMount = FindThreatTarget(bot, "grauf");

    if (!bossMount)
    // Actual bossfight (dismounted)
    {
        if (boss->HasAura(SPELL_SKADI_WHIRLWIND))
        {
            if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AvoidSkadiWhirlwindAction*>(action))
            {
                return 0.0f;
            }
        }
    }
    else
    {
        // Bots tend to get stuck trying to attack the boss in the sky, not the adds on the ground
        if (dynamic_cast<AttackAction*>(action)
            && (action->GetTarget() == boss || action->GetTarget() == bossMount))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

float YmironMultiplier::GetValue(Action* action)
{
    Unit* boss = FindThreatTarget(bot, "king ymiron");
    if (!boss) { return 1.0f; }

    if (boss->FindCurrentSpellBySpellId(SPELL_BANE) || boss->HasAura(SPELL_BANE))
    {
        if (dynamic_cast<AttackAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}
