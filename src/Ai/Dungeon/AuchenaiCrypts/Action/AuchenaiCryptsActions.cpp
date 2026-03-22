#include "Playerbots.h"
#include "AiFactory.h"
#include "AuchenaiCryptsTriggers.h"
#include "AuchenaiCryptsActions.h"

// Shirrak the Dead Watcher

static const Position SHIRRAK_TANK_POSITION = { -53.898f, -163.214f, 26.389f };
// static const Position STAIRS_TOP_POSITION = { -17.170f, -162.580f, 26.013f };

// Tank will position Shirrak at the specified coordinates

bool ShirrakTankPositionBossAction::Execute(Event /*event*/) 
{
    Unit* shirrak = AI_VALUE2(Unit*, "find target", "shirrak the dead watcher");
    if (!shirrak)
        return false;

    if (bot->GetVictim() != shirrak)
        return Attack(shirrak);

    if (shirrak->GetVictim() == bot && bot->IsWithinMeleeRange(shirrak))
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

//Flee from Shirrak's Focus Fire

bool FleeFocusFireAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 20.0f;
        std::list<Creature*> creatureList;
        bot->GetCreatureListWithEntryInGrid(creatureList, static_cast<uint32>(AuchenaiCryptsIDs::NPC_FOCUS_FIRE), 20.0f);

    for (Creature* flare : creatureList)
    {
        if (flare && flare->IsAlive())
        {
            float currentDistance = bot->GetDistance2d(flare);
            constexpr float safeDistance = 20.0f; 
            constexpr float buffer = 5.0f;

            if (currentDistance < safeDistance)
            {
                bot->AttackStop();
                bot->InterruptNonMeleeSpells(true);

                float distanceToMove = safeDistance - currentDistance + buffer;

                return MoveAway(flare, distanceToMove);
            }
        }
    }
    return false;
}
