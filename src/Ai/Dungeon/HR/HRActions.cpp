#include "Playerbots.h"
#include "PlayerbotAI.h"
#include "AiFactory.h"
#include "HRTriggers.h"
#include "HRActions.h"
#include "MovementActions.h"
#include "RaidBossHelpers.h"

constexpr uint32 HR_MAP_ID = 543;

// Watchkeeper Gargolmar

static const Position GARGOLMAR_TANK_POSITION = { -1196.097f, 1439.785f, 68.500f };

// Tank will position Gargolmar at the specified coordinates to minimize risk of chaining other packs.
bool GargolmarTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* gargolmar = AI_VALUE2(Unit*, "find target", "watchkeeper gargolmar");
    if (!gargolmar)
        return false;

    if (bot->GetVictim() != gargolmar)
        return Attack(gargolmar);

    if (gargolmar->GetVictim() == bot && bot->IsWithinMeleeRange(gargolmar) &&
        bot->GetHealthPct()>30.0f)
    {
        const Position& position = GARGOLMAR_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(),
                                                   position.GetPositionY());
        if (distToPosition > 6.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(2.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false,
                   false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Hellfire Watchers will be marked with skull

bool GargolmarMarkHellfireWatchersAction::Execute(Event /*event*/)
{
    Unit* watcher = AI_VALUE2(Unit*, "find target", "hellfire watcher");
    if (!watcher)
        return false;

    if (IsMechanicTrackerBot(botAI, bot, HR_MAP_ID, nullptr))
            MarkTargetWithSkull(bot, watcher);

    SetRtiTarget(botAI, "skull", watcher);

    return false;
}

// Omor the Unscarred

// Flee 15 yards from other players if you have Treacherous Aura
bool OmorTreacherousAuraFleeFromPlayersAction::Execute(Event /*event*/)
{
    constexpr float safeDistance = 15.0f;
    if (GetNearestPlayerInRadius(bot, safeDistance))
    {
        botAI->Reset();
        return MoveFromGroup(safeDistance);
    }

    return false;
}

// Flee 15 yards from other players if you have Bane of Treachery
bool OmorBaneOfTreacheryAuraFleeFromPlayersAction::Execute(Event /*event*/)
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
    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance);

    if (nearestPlayer)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
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

    if (IsMechanicTrackerBot(botAI, bot, HR_MAP_ID, nullptr))
            MarkTargetWithSkull(bot, hound);

    SetRtiTarget(botAI, "skull", hound);

    return false;
}

// Future logic for Tank getting hit with Treacherous Aura or Bane of Treachery

// Vazruden

static const Position VAZRUDEN_TANK_POSITION = { -1407.405, 1744.521, 81.075 };

// Tank positions Vazruden on the middle of the platform (for some reason bots try to grab the dragon flying around the platform. This is to help prevent that.)
bool VazrudenTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* vazruden = AI_VALUE2(Unit*, "find target", "vazruden");
    if (!vazruden)
        return false;

    if (bot->GetVictim() != vazruden)
        return Attack(vazruden);

    if (vazruden->GetVictim() == bot && bot->IsWithinMeleeRange(vazruden) &&
        bot->GetHealthPct()>30.0f)
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

            return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false,
                   false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}