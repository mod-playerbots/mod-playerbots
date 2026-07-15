/*
* This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
* information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License, or (at your option) any later version.
*/

#include "Playerbots.h"
#include "HFRTriggers.h"
#include "HFRActions.h"
#include "RaidBossHelpers.h"

constexpr uint32 HFR_MAP_ID = 543;

// Watchkeeper Gargolmar

// Hellfire Watchers will be marked with skull

bool GargolmarMarkHellfireWatchersAction::Execute(Event /*event*/)
{
    Unit* watcher = AI_VALUE2(Unit*, "find target", "hellfire watcher");
    if (!watcher)
        return false;

    if (IsMechanicTrackerBot(botAI, bot, HFR_MAP_ID, nullptr))
        MarkTargetWithSkull(bot, watcher);

    SetRtiTarget(botAI, "skull", watcher);

    return false;
}

// Omor the Unscarred

// Flee 15 yards from other players if you have Treacherous Aura or Bane of Treachery
bool OmorTreacheryAuraFleeFromPlayersAction::Execute(Event /*event*/)
{
    constexpr float safeDistance = 15.0f;
    if (GetNearestPlayerInRadius(bot, safeDistance))
    {
        botAI->Reset();
        return MoveFromGroup(safeDistance);
    }

    return false;
}

// ranged spread out 15 yards from each other
bool OmorRangedSpreadAction::Execute(Event /*event*/)
{
    const float minDistance = 15.0f;

    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance))
    {
        return FleePosition(nearestPlayer->GetPosition(), minDistance);
    }

    return false;
}

// Mark Fiendish Hound with skull
bool OmorMarkFiendishHoundAction::Execute(Event /*event*/)
{
    Unit* hound = AI_VALUE2(Unit*, "find target", "fiendish hound");
    if (!hound)
        return false;

    if (IsMechanicTrackerBot(botAI, bot, HFR_MAP_ID, nullptr))
        MarkTargetWithSkull(bot, hound);

    SetRtiTarget(botAI, "skull", hound);

    return false;
}

// Nearby bots should flee 15 yards from the tank if it has Treacherous Aura or Bane of Treachery
bool OmorTreacheryAuraFleeFromTankAction::Execute(Event /*event*/)
{
    Player* tank = GetGroupMainTank(botAI, bot);
    if (!tank)
        return false;

    constexpr float safeDistance = 15.0f;
    constexpr float buffer = 1.0f;

    if (bot->GetDistance2d(tank) < safeDistance)
    {
        botAI->Reset();
        return MoveAway(tank, safeDistance + buffer);
    }

    return false;
}

// Vazruden

static const Position VAZRUDEN_TANK_POSITION = { -1407.405, 1744.521, 81.075 };

// Tank positions Vazruden on the middle of the platform (for some reason bots try to grab the dragon flying around the platform. This is to help prevent that.)
bool VazrudenTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* vazruden = AI_VALUE2(Unit*, "find target", "vazruden");
    if (!vazruden)
        return false;

    if (AI_VALUE(Unit*, "current target") != vazruden)
        return Attack(vazruden);

    if (vazruden->GetVictim() == bot && bot->IsWithinMeleeRange(vazruden) &&
        bot->GetHealthPct() > 30.0f)
    {
        const Position& position = VAZRUDEN_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(),
                                                   position.GetPositionY());
        if (distToPosition > 6.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(2.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(HFR_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                   false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool VazrudenMarkBossAction::Execute(Event /*event*/)
{
    Unit* vaz = AI_VALUE2(Unit*, "find target", "vazruden");
    if (!vaz)
        return false;

    if (IsMechanicTrackerBot(botAI, bot, HFR_MAP_ID, nullptr))
        MarkTargetWithSkull(bot, vaz);

    SetRtiTarget(botAI, "skull", vaz);

    return false;
}
