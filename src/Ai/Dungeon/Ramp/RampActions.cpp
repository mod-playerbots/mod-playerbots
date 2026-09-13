/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RampActions.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"

using namespace EncounterHelpers;

constexpr uint32 RAMP_MAP_ID = 543;
static const Position VAZRUDEN_TANK_POSITION = {-1407.405f, 1744.521f, 81.075f};

// Watchkeeper Gargolmar

// Hellfire Watchers will be marked with skull

bool GargolmarMarkHellfireWatchersAction::Execute(Event /*event*/)
{
    Unit* watcher = AI_VALUE2(Unit*, "find target", "hellfire watcher");
    if (!watcher)
        return false;

    if (!IsMechanicTrackerBot(bot, RAMP_MAP_ID))
        return false;

    return MarkTargetWithSkull(bot, watcher);
}

// Omor the Unscarred

// Flee 20 yards from other players if you have Treacherous Aura or Bane of Treachery
bool OmorTreacheryAuraFleeFromPlayersAction::Execute(Event /*event*/)
{
    constexpr float safeDistance = 20.0f;

    if (!GetNearestPlayerInRadius(bot, safeDistance))
        return false;

    bot->CastStop();

    return MoveFromGroup(safeDistance);
}

// Nearby bots should flee 20 yards from the tank if it has Treacherous Aura or Bane of Treachery
bool OmorTreacheryAuraFleeFromTankAction::Execute(Event /*event*/)
{
    Unit* omor = AI_VALUE2(Unit*, "find target", "omor the unscarred");

    if (!omor)
        return false;

    Unit* omorVictim = omor->GetVictim();

    if (!omorVictim || omorVictim->GetGUID() == bot->GetGUID())
        return false;

    constexpr float safeDistance = 20.0f;

    if (bot->GetExactDist2d(omorVictim) >= safeDistance)
        return false;

    bot->CastStop();
    return MoveAway(omorVictim, safeDistance);
}

// Ranged spread out 20 yards from each other
bool OmorRangedSpreadAction::Execute(Event /*event*/)
{
    constexpr float minDistance = 20.0f;

    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance))
        return FleePosition(nearestPlayer->GetPosition(), minDistance);

    return false;
}

// Mark Fiendish Hound with skull
bool OmorMarkFiendishHoundAction::Execute(Event /*event*/)
{
    Unit* hound = AI_VALUE2(Unit*, "find target", "fiendish hound");
    if (!hound)
        return false;

    if (!IsMechanicTrackerBot(bot, RAMP_MAP_ID))
        return false;

    return MarkTargetWithSkull(bot, hound);
}

// Vazruden

// Tank positions Vazruden on the middle of the platform (for some reason bots try to grab the dragon flying around the
// platform. This is to help prevent that.)
bool VazrudenTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* vazruden = AI_VALUE2(Unit*, "find target", "vazruden");
    if (!vazruden)
        return false;

    if (AI_VALUE(Unit*, "current target") != vazruden)
        return Attack(vazruden);

    if (vazruden->GetVictim() != bot || !bot->IsWithinMeleeRange(vazruden) || bot->GetHealthPct() <= 30.0f)
        return false;

    Position const& position = VAZRUDEN_TANK_POSITION;
    constexpr float arrivalDist = 6.0f;
    float distToPosition = bot->GetExactDist2d(position);

    if (distToPosition <= arrivalDist)
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, vazruden, moveX, moveY, backwards))
        return false;

    return MoveTo(RAMP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool VazrudenMarkBossAction::Execute(Event /*event*/)
{
    Unit* vazruden = AI_VALUE2(Unit*, "find target", "vazruden");
    if (!vazruden)
        return false;

    if (!IsMechanicTrackerBot(bot, RAMP_MAP_ID))
        return false;

    return MarkTargetWithSkull(bot, vazruden);
}
