#include "Playerbots.h"
#include "AiFactory.h"
#include "HRTriggers.h"
#include "HRActions.h"

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

    MarkTargetWithSkull(bot, watcher);
    SetRtiTarget(botAI, "skull", watcher);

    return true;
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
