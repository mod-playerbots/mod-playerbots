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

// ranged spread out 15 yards from each other
bool OmorRangedSpreadAction::Execute(Event /*event*/)
{
    Unit* omor = AI_VALUE2(Unit*, "find target", "omor the unscarred");
    if (!omor)
        return false;

    std::vector<Player*> rangedBots;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && botAI->IsRanged(member))
                rangedBots.push_back(member);
        }
    }

    auto findIt = std::find(rangedBots.begin(), rangedBots.end(), bot);
    if (findIt == rangedBots.end())
        return false;

    size_t botIndex = std::distance(rangedBots.begin(), findIt);
    size_t count = rangedBots.size();

    float angle = (count <= 1) ? 0.0f : ((2.0f * M_PI) * (float)botIndex / (float)count);

    constexpr float spreadRadius = 18.0f;
    float targetX = omor->GetPositionX() + cos(angle) * spreadRadius;
    float targetY = omor->GetPositionY() + sin(angle) * spreadRadius;

    float distToSpot = bot->GetExactDist2d(targetX, targetY);

    if (distToSpot > 3.0f)
    {
        float dX = targetX - bot->GetPositionX();
        float dY = targetY - bot->GetPositionY();

        float moveDist = std::min(2.0f, distToSpot);
        float moveX = bot->GetPositionX() + (dX / distToSpot) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToSpot) * moveDist;

        return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}
