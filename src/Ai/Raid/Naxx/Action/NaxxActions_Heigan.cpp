/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */


#include "NaxxActions.h"
#include "Playerbots.h"
#include "Timer.h"

bool HeiganDanceAction::calculatesafe()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "heigan the unclean");
    if (!boss)
    {
        return false;
    }
    uint32 now = getMSTime();
    bool current_platform_phase = boss->IsWithinDist2d(platform.first, platform.second, 10.0f);
    if (combat_start_ms == 0)
    {
        combat_start_ms = now;
    }

    if ((current_platform_phase != last_platform_phase))
    {
        resetsafe();
    }
    platform_phase = current_platform_phase;
    last_platform_phase = current_platform_phase;

    if ((last_eruption_ms == 0 || now - last_eruption_ms > 3000) && (now - combat_start_ms > 12000))
    {
        bool foundEruption = false;

        GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        for (auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (!unit)
            {
                continue;
            }
            if (unit->GetEntry() != 12999)
            {
                continue;
            }
            foundEruption = true;
            break;
        }

        if (foundEruption)
        {
            nextsafe();
            last_eruption_ms = now;
        }
    }

    return true;
}

bool HeiganDanceMeleeAction::Execute(Event event)
{
    calculatesafe();
    if (!platform_phase && botAI->IsMainTank(bot) && !AI_VALUE2(bool, "has aggro", "boss target"))
    {
        return false;
    }
    assert(curr_safe >= 0 && curr_safe <= 3);
    float safeX = waypoints[curr_safe].first;
    float safeY = waypoints[curr_safe].second;
    float safeRadius = botAI->IsMainTank(bot) ? 0.5f : 6.0f;

    if (!platform_phase && bot->IsWithinDist2d(safeX, safeY, safeRadius))
    {
        return false;
    }

    return MoveInside(bot->GetMapId(), waypoints[curr_safe].first, waypoints[curr_safe].second, bot->GetPositionZ(),
                      botAI->IsMainTank(bot) ? 0 : 0, MovementPriority::MOVEMENT_COMBAT);
}

bool HeiganDanceRangedAction::Execute(Event event)
{
    calculatesafe();
    if (!platform_phase)
    {
        if (bot->IsWithinDist2d(platform.first, platform.second, 1.5f))
        {
            return false;  
        }

        if (MoveTo(bot->GetMapId(), platform.first, platform.second, 276.54f, false, false, false, false,
                   MovementPriority::MOVEMENT_COMBAT))
        {
            return true;
        }
        return MoveInside(bot->GetMapId(), platform.first, platform.second, 276.54f, 2.0f,
                          MovementPriority::MOVEMENT_COMBAT);
    }
    botAI->InterruptSpell();
    return MoveInside(bot->GetMapId(), waypoints[curr_safe].first, waypoints[curr_safe].second, bot->GetPositionZ(), 0,
                      MovementPriority::MOVEMENT_COMBAT);
}
