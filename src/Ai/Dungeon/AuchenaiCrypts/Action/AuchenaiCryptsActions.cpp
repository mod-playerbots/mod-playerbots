#include "Playerbots.h"
#include "AiFactory.h"
#include "AuchenaiCryptsTriggers.h"
#include "AuchenaiCryptsActions.h"

// Shirrak the Dead Watcher

static const Position SHIRRAK_RANGED_POSITION = { -21.777f, -162.700f, 26.062f };
static const Position SHIRRAK_TANK_POSITION = { -65.171f, -162.920f, 26.504f };

// Tank will position Shirrak at the specified coordinates, further down the corridor past the stairs

bool ShirrakTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* shirrak = AI_VALUE2(Unit*, "find target", "shirrak the dead watcher");
    if (!shirrak)
        return false;

    if (bot->GetVictim() != shirrak)
        return Attack(shirrak);

    if (shirrak->GetVictim() == bot && bot->IsWithinMeleeRange(shirrak) &&
        bot->GetHealthPct()>30.0f)
    {
        const Position& position = SHIRRAK_TANK_POSITION;
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

//  Flee from Shirrak's Focus Fire

bool ShirrakFleeFocusFireAction::Execute(Event /*event*/)
{
    std::list<Creature*> creatureList;
        bot->GetCreatureListWithEntryInGrid(creatureList, static_cast<uint32>(AuchenaiCryptsIDs::NPC_FOCUS_FIRE), 20.0f);

    for (Creature* flare : creatureList)
    {
        if (flare && flare->IsAlive())
        {
            float currentDistance = bot->GetDistance2d(flare);
            constexpr float safeDistance = 12.0f;
            constexpr float buffer = 5.0f;

            if (currentDistance < safeDistance)
            {
                bot->AttackStop();

                float distanceToMove = safeDistance - currentDistance + buffer;

                return MoveAway(flare, distanceToMove);
            }
        }
    }
    return false;
}

// Ranged should keep distance from Shirrak, staying at the edge of the stairs

bool ShirrakRangedKeepDistanceAction::Execute(Event /*event*/)
{

    std::vector<Player*> rangedBots;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && botAI->IsRanged(member))
                rangedBots.push_back(member);
        }
    }

    auto findIt = std::find(rangedBots.begin(), rangedBots.end(), bot);
    size_t botIndex = (findIt != rangedBots.end()) ? std::distance(rangedBots.begin(), findIt) : 0;
    size_t count = rangedBots.size();

    constexpr float arcSpan = M_PI / 2.0f;
    float arcCenter = M_PI;
    float arcStart = arcCenter - (arcSpan / 2.0f);

    float angle = (count <= 1) ? arcCenter : (arcStart + (arcSpan * (float)botIndex / (float)(count - 1)));

    constexpr float spreadRadius = 3.0f;
    float targetX = SHIRRAK_RANGED_POSITION.GetPositionX() + cos(angle) * spreadRadius;
    float targetY = SHIRRAK_RANGED_POSITION.GetPositionY() + sin(angle) * spreadRadius;

    float distToSpot = bot->GetExactDist2d(targetX, targetY);

    if (distToSpot > 4.0f)
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
