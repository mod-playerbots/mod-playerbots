/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <vector>

#include "SWPActions.h"
#include "SWPEncounter_Twins.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace SunwellHelpers;

bool EredarTwinsMeleeJumpDownFromBalconyAction::Execute(Event /*event*/)
{
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    const Position& jumpPos = EREDAR_TWINS_P1_RANGED_POSITION;
    const Position landingPos = GetEredarTwinsP2MeleeStackPosition(alythess);

    constexpr float arrivalDistance = 2.0f;
    const float distanceToJumpPos = bot->GetExactDist2d(
        jumpPos.GetPositionX(), jumpPos.GetPositionY());

    if (distanceToJumpPos > arrivalDistance)
    {
        return MoveTo(
            SUNWELL_MAP_ID, jumpPos.GetPositionX(), jumpPos.GetPositionY(),
            jumpPos.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else
    {
        return JumpTo(
            SUNWELL_MAP_ID, landingPos.GetPositionX(), landingPos.GetPositionY(),
            landingPos.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
    }
}

bool EredarTwinsMisdirectBossesToTanksAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
            GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }

        if (hunters.size() >= 3)
            break;
    }

    int8 hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int8>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    Unit* bossTarget = nullptr;
    Player* tankTarget = nullptr;
    if (hunterIndex == 0)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
        tankTarget = GetGroupAssistTank(botAI, bot, 0);
    }
    else if (hunterIndex == 1)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "lady sacrolash");
        tankTarget = GetGroupMainTank(botAI, bot);
    }
    else if (hunterIndex == 2)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "lady sacrolash");
        tankTarget = GetGroupAssistTank(botAI, bot, 1);
    }

    if (!tankTarget || !tankTarget->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", bossTarget))
    {
        return botAI->CastSpell("steady shot", bossTarget);
    }

    return false;
}

bool EredarTwinsMainAndSecondAssistTanksPositionSacrolashAction::Execute(Event /*event*/)
{
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (!sacrolash)
        return false;

    if (AI_VALUE(Unit*, "current target") != sacrolash)
        return Attack(sacrolash);

    if (sacrolash->GetVictim() == bot && bot->IsWithinMeleeRange(sacrolash))
    {
        const Position& position = SACROLASH_TANK_POSITION;
        const float distToPosition = bot->GetExactDist2d(
            position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 2.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(2.25f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(
                SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool EredarTwinsFirstAssistTankMoveOutOfBlazeAction::Execute(Event /*event*/)
{
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    if (!alythess)
        return false;

    uint8 index = _alythessTankStep;
    if (index >= ALYTHESS_TANK_POSITION_COUNT || bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
        index = 0;

    if (AI_VALUE(Unit*, "current target") != alythess)
        return Attack(alythess);

    auto const findSafeAlythessTankIndex =
        [&](uint8 startIndex, bool includeStart, uint8& safeIndex)
    {
        const size_t offsetStart = includeStart ? 0 : 1;
        for (size_t offset = offsetStart; offset < ALYTHESS_TANK_POSITION_COUNT; ++offset)
        {
            const uint8 candidateIndex =
                static_cast<uint8>((startIndex + offset) % ALYTHESS_TANK_POSITION_COUNT);

            if (IsAlythessTankPositionSafe(bot, GetAlythessTankPosition(alythess, candidateIndex)))
            {
                safeIndex = candidateIndex;
                return true;
            }
        }

        return false;
    };

    if (!IsAlythessTankPositionSafe(bot, GetAlythessTankPosition(alythess, index)))
    {
        uint8 safeIndex = index;
        if (!findSafeAlythessTankIndex(index, false, safeIndex))
            return false;

        index = safeIndex;
        _alythessTankStep = index;
    }

    const Position position = GetAlythessTankPosition(alythess, index);

    constexpr float maxDistance = 1.0f;
    const float distToPosition = bot->GetExactDist2d(position);

    if (alythess->GetVictim() == bot)
    {
        if (distToPosition <= maxDistance && ShouldAdvanceAlythessTankPosition(alythess, bot))
        {
            uint8 safeIndex = index;
            if (!findSafeAlythessTankIndex(index, false, safeIndex))
                return false;

            index = safeIndex;
            _alythessTankStep = index;
            const Position newPosition = GetAlythessTankPosition(alythess, index);
            const float newDistToPosition = bot->GetExactDist2d(newPosition);

            if (newDistToPosition > maxDistance)
            {
                const float dX = newPosition.GetPositionX() - bot->GetPositionX();
                const float dY = newPosition.GetPositionY() - bot->GetPositionY();
                const float moveDist = std::min(3.5f, newDistToPosition);
                const float moveX = bot->GetPositionX() + (dX / newDistToPosition) * moveDist;
                const float moveY = bot->GetPositionY() + (dY / newDistToPosition) * moveDist;

                return MoveTo(
                    SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                    false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
        else if (distToPosition > maxDistance)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(3.5f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(
                SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool EredarTwinsPositionRangedAction::Execute(Event /*event*/)
{
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (sacrolash && sacrolash->GetVictim() != bot)
    {
        const Position& position = EREDAR_TWINS_P1_RANGED_POSITION;

        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 1.0f)
        {
            return MoveTo(
                SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                position.GetPositionZ(), false, false, false, false,
                MovementPriority::MOVEMENT_FORCED, true, false);
        }

        return false;
    }
    // Jump down during Phase 2 or if the bot pulls aggro on Sacrolash
    else if (bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
    {
        Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
        const Position& jumpPos = EREDAR_TWINS_P1_RANGED_POSITION;
        const Position landingPos = GetEredarTwinsP2RangedStackPosition(alythess);

        constexpr float arrivalDistance = 2.0f;
        const float distanceToJumpPos = bot->GetExactDist2d(
            jumpPos.GetPositionX(), jumpPos.GetPositionY());

        if (distanceToJumpPos > arrivalDistance)
        {
            return MoveTo(
                SUNWELL_MAP_ID, jumpPos.GetPositionX(), jumpPos.GetPositionY(),
                jumpPos.GetPositionZ(), false, false, false, false,
                MovementPriority::MOVEMENT_FORCED, true, false);
        }
        else
        {
            return JumpTo(
                SUNWELL_MAP_ID, landingPos.GetPositionX(), landingPos.GetPositionY(),
                landingPos.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool EredarTwinsStackInRoomCenterAction::Execute(Event /*event*/)
{
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    if (!alythess)
        return false;

    const Position position = botAI->IsRanged(bot) ?
        GetEredarTwinsP2RangedStackPosition(alythess) :
        GetEredarTwinsP2MeleeStackPosition(alythess);

    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 0.5f)
    {
        return MoveTo(
            SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
            position.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool EredarTwinsRemoveFlameSearAction::Execute(Event /*event*/)
{
    switch (bot->getClass())
    {
        case CLASS_MAGE:
            return botAI->CanCastSpell("ice block", bot) &&
                botAI->CastSpell("ice block", bot);

        case CLASS_PALADIN:
            return botAI->CanCastSpell("divine shield", bot) &&
                botAI->CastSpell("divine shield", bot);

        case CLASS_ROGUE:
            return botAI->CanCastSpell("cloak of shadows", bot) &&
                botAI->CastSpell("cloak of shadows", bot);

        default:
            return false;
    }
}

bool EredarTwinsDpsPrioritizeLadySacrolashAction::Execute(Event /*event*/)
{
    if (Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash"))
    {
        constexpr float threatRatio = 0.8f;
        if (ShouldHoldTwinThreat(botAI, bot, sacrolash, threatRatio, IsAnySacrolashTank))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            bot->SetTarget(ObjectGuid::Empty);
            bot->SetSelection(ObjectGuid());
            return true;
        }
        else if (AI_VALUE(Unit*, "current target") != sacrolash)
        {
            return Attack(sacrolash);
        }

        return false;
    }
    else if (Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
    {
        constexpr float threatRatio = 0.9f;
        if (ShouldHoldTwinThreat(botAI, bot, alythess, threatRatio, IsAlythessTank))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            bot->SetTarget(ObjectGuid::Empty);
            bot->SetSelection(ObjectGuid());
            return true;
        }
        else if (AI_VALUE(Unit*, "current target") != alythess)
        {
            return Attack(alythess);
        }
    }

    return false;
}

bool EredarTwinsConflagratedBotMoveFromGroupAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_ROGUE && botAI->CanCastSpell("vanish", bot) &&
        botAI->CastSpell("vanish", bot))
    {
        return true;
    }

    if (AI_VALUE2(Unit*, "find target", "lady sacrolash"))
    {
        const Position& position = botAI->IsRanged(bot) ?
            EREDAR_TWINS_RANGED_CONFLAG_POSITION : EREDAR_TWINS_MELEE_CONFLAG_POSITION;

        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 1.0f)
        {
            return MoveTo(
                SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                position.GetPositionZ(), false, false, false, false,
                MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }
    else
    {
        constexpr float safeDistance = 10.0f;
        if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance))
        {
            const float distanceToPlayer = bot->GetExactDist2d(nearestPlayer);
            if (distanceToPlayer < safeDistance)
            {
                botAI->InterruptSpell();
                return MoveAway(nearestPlayer, safeDistance - distanceToPlayer);
            }
        }
    }

    return false;
}

bool EredarTwinsMoveFromConflagSacrolashVictimAction::Execute(Event /*event*/)
{
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (!sacrolash)
        return false;

    Unit* victim = sacrolash->GetVictim();
    if (!victim)
        return false;

    constexpr float safeDistance = 10.0f;
    if (bot->GetDistance2d(victim) < safeDistance)
    {
        botAI->InterruptSpell();
        return MoveFromGroup(safeDistance);
    }

    return false;
}
