/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RampActions.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "RampShared.h"

using namespace EncounterHelpers;
using namespace RampShared;

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

    Unit* tank = GetGroupMainTank(bot);

    if (!tank)
        return false;

    constexpr float safeDistance = 20.0f;

    if (bot->GetExactDist2d(tank) >= safeDistance)
        return false;

    bot->CastStop();
    return MoveAway(tank, safeDistance);
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

// Tank Omor towards the middle of the platform
bool OmorTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* omor = AI_VALUE2(Unit*, "find target", "omor the unscarred");
    if (!omor)
        return false;

    if (omor->GetVictim() != bot || !bot->IsWithinMeleeRange(omor) || bot->GetHealthPct() <= 25.0f)
        return false;

    Position const& position = OMOR_TANK_POSITION;
    constexpr float arrivalDist = 3.0f;
    float distToPosition = bot->GetExactDist2d(position);

    if (distToPosition <= arrivalDist)
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, omor, moveX, moveY, backwards))
        return false;

    return MoveTo(RAMP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Vazruden & Nazan

// Tank Vazruden in the middle of the platform
bool VazrudenTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* vazruden = AI_VALUE2(Unit*, "find target", "vazruden");
    if (!vazruden)
        return false;

    if (AI_VALUE(Unit*, "current target") != vazruden)
        return Attack(vazruden);

    if (vazruden->GetVictim() != bot || !bot->IsWithinMeleeRange(vazruden) || bot->GetHealthPct() <= 25.0f)
        return false;

    Position const& position = VAZRUDEN_TANK_POSITION;
    constexpr float arrivalDist = 10.0f;
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

// Mark Vazruden with 'Skull'
bool VazrudenMarkBossAction::Execute(Event /*event*/)
{
    Unit* vazruden = AI_VALUE2(Unit*, "find target", "vazruden");
    if (!vazruden ||
        !IsMechanicTrackerBot(bot, RAMP_MAP_ID) ||
        !vazruden->IsAlive())
        return false;

    return MarkTargetWithSkull(bot, vazruden);
}

// Shamans use Tremor totem when Nazan is active
bool NazanSetTremorTotemAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell(Id(RampSpells::SPELL_TREMOR_TOTEM), bot) &&
           botAI->CastSpell(Id(RampSpells::SPELL_TREMOR_TOTEM), bot) &&
           !AI_VALUE2(bool, "has totem", "tremor totem");
}

// Shamans use Fire Resistance totem when Nazan is active
bool NazanSetFireResistanceTotemAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell(Id(RampSpells::SPELL_FIRE_RESISTANCE_TOTEM_RANK_1), bot) &&
           botAI->CastSpell(Id(RampSpells::SPELL_FIRE_RESISTANCE_TOTEM_RANK_1), bot) &&
           !AI_VALUE2(bool, "has totem", "fire resistance totem");
}
