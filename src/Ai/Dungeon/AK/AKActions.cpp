/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AKActions.h"
#include "AKTriggers.h"
#include "Playerbots.h"
#include "ThreatHelpers.h"

bool AttackNadoxGuardianAction::Execute(Event /*event*/)
{
    Unit* target = FindThreatTarget(bot, "ahn'kahar guardian");
    if (!target || AI_VALUE(Unit*, "current target") == target)
    {
        return false;
    }
    return Attack(target);
}

bool AttackJedogaVolunteerAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_TWILIGHT_VOLUNTEER)
        {
            target = unit;
            break;
        }
    }

    if (!target || AI_VALUE(Unit*, "current target") == target)
    {
        return false;
    }
    return Attack(target);
}

bool AvoidShadowCrashAction::Execute(Event /*event*/)
{
    // Could check all enemy units in range as it's possible to pull multiple of these mobs.
    // They should really be killed 1 by 1, multipulls are messy so we just handle singles for now
    Unit* unit = FindThreatTarget(bot, "forgotten one");
    if (!unit) { return false; }

    Unit* victim = nullptr;
    float radius = 10.0f;
    float targetDist = radius + 2.0f;

    // Actively move if targeted by a shadow crash.
    // Spell check not needed, they don't have any other non-instant casts
    if (unit->HasUnitState(UNIT_STATE_CASTING))
    {
        // This doesn't seem to avoid casts very well, perhaps because this isn't checked while allies are casting.
        victim = botAI->GetUnit(unit->GetTarget());
        float distance = bot->GetExactDist2d(victim->GetPosition());

        if (victim && distance < radius)
        {
            return MoveAway(victim, targetDist - distance);
        }
    }

    // Otherwise ranged members passively spread, to avoid AoE overlap
    if (PlayerbotAI::IsMelee(bot)) { return false; }

    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        Unit* unit = botAI->GetUnit(member);
        if (!unit || bot->GetGUID() == member)
        {
            continue;
        }
        float currentDist = bot->GetExactDist2d(botAI->GetUnit(member));
        if (currentDist < radius)
        {
            return MoveAway(botAI->GetUnit(member), targetDist - currentDist);
        }
    }
    return false;
}
