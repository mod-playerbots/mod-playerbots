/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "SWPEncounter_Twins.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include <vector>

using namespace SwpHelpers;

bool EredarTwinsMeleeJumpDownFromBalconyAction::Execute(Event /*event*/)
{
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    Position const& jumpPosition = EREDAR_TWINS_P1_RANGED_POSITION;
    Position const landingPosition = GetEredarTwinsP2MeleePosition(alythess);

    constexpr float arrivalDistance = 2.0f;
    float const distanceToJumpPos = bot->GetExactDist2d(jumpPosition);

    if (distanceToJumpPos > arrivalDistance)
    {
        return MoveTo(
            SWP_MAP_ID, jumpPosition.GetPositionX(), jumpPosition.GetPositionY(),
            jumpPosition.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else
    {
        return JumpTo(
            SWP_MAP_ID, landingPosition.GetPositionX(), landingPosition.GetPositionY(),
            landingPosition.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
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
        if (member && member->IsAlive() && member->GetMapId() == SWP_MAP_ID &&
            member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
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

    if (bot->HasAura(Id(SwpSpells::SPELL_MISDIRECTION)) &&
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

    if (sacrolash->GetVictim() != bot || !bot->IsWithinMeleeRange(sacrolash))
        return false;

    Position const& position = SACROLASH_TANK_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);
    if (distToPosition <= 2.0f)
        return false;

    float const posX = position.GetPositionX();
    float const posY = position.GetPositionY();
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    float const toPosX = posX - botX;
    float const toPosY = posY - botY;
    float const toBossX = sacrolash->GetPositionX() - botX;
    float const toBossY = sacrolash->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool EredarTwinsFirstAssistTankMoveOutOfBlazeAction::Execute(Event /*event*/)
{
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    if (!alythess)
        return false;

    if (AI_VALUE(Unit*, "current target") != alythess)
        return Attack(alythess);

    auto const findSafeAlythessTankIndex =
        [&](uint8 startIndex, bool includeStart, uint8& safeIndex)
    {
        size_t const offsetStart = includeStart ? 0 : 1;
        for (size_t offset = offsetStart; offset < ALYTHESS_TANK_POSITION_COUNT; ++offset)
        {
            uint8 const candidateIndex =
                static_cast<uint8>((startIndex + offset) % ALYTHESS_TANK_POSITION_COUNT);

            if (IsAlythessTankPositionSafe(bot, GetAlythessTankPosition(alythess, candidateIndex)))
            {
                safeIndex = candidateIndex;
                return true;
            }
        }

        return false;
    };

    uint8 index = _alythessTankStep;

    if (!IsAlythessTankPositionSafe(bot, GetAlythessTankPosition(alythess, index)))
    {
        uint8 safeIndex = index;
        if (!findSafeAlythessTankIndex(index, false, safeIndex))
            return false;

        index = safeIndex;
        _alythessTankStep = index;
    }

    Position const position = GetAlythessTankPosition(alythess, index);
    constexpr float maxDistance = 1.0f;
    float const distToPosition = bot->GetExactDist2d(position);

    if (alythess->GetVictim() != bot)
        return false;

    if (distToPosition <= maxDistance && ShouldAdvanceAlythessTankPosition(alythess, bot))
    {
        uint8 safeIndex = index;
        if (!findSafeAlythessTankIndex(index, false, safeIndex))
            return false;

        index = safeIndex;
        _alythessTankStep = index;
        Position const newPos = GetAlythessTankPosition(alythess, index);

        if (bot->GetExactDist2d(newPos) > maxDistance)
        {
            return MoveTo(
                SWP_MAP_ID, newPos.GetPositionX(), newPos.GetPositionY(), newPos.GetPositionZ(),
                false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }
    else if (distToPosition > maxDistance)
    {
        return MoveTo(
            SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool EredarTwinsPositionRangedAction::Execute(Event /*event*/)
{
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (sacrolash && sacrolash->GetVictim() != bot && GetEredarTwinsBlazeTarget(bot) != bot)
    {
        Position const& position = EREDAR_TWINS_P1_RANGED_POSITION;
        if (bot->GetExactDist2d(position) <= 1.0f)
            return false;

        return MoveTo(
            SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }
    // Jump down during Phase 2 or if the bot pulls aggro on Sacrolash
    else if (bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
    {
        Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
        Position const& jumpPosition = EREDAR_TWINS_P1_RANGED_POSITION;
        Position const landingPosition = GetEredarTwinsP2RangedPosition(alythess);

        constexpr float arrivalDistance = 2.0f;
        float const distanceToJumpPos = bot->GetExactDist2d(jumpPosition);

        if (distanceToJumpPos > arrivalDistance)
        {
            return MoveTo(
                SWP_MAP_ID, jumpPosition.GetPositionX(), jumpPosition.GetPositionY(),
                jumpPosition.GetPositionZ(), false, false, false, false,
                MovementPriority::MOVEMENT_FORCED, true, false);
        }
        else
        {
            return JumpTo(
                SWP_MAP_ID, landingPosition.GetPositionX(), landingPosition.GetPositionY(),
                landingPosition.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool EredarTwinsStackInRoomCenterAction::Execute(Event /*event*/)
{
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    if (!alythess)
        return false;

    Position const position = PlayerbotAI::IsRanged(bot) ?
        GetEredarTwinsP2RangedPosition(alythess) :
        GetEredarTwinsP2MeleePosition(alythess);

    if (bot->GetExactDist2d(position) <= 0.5f)
        return false;

    return MoveTo(
        SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
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
        if (ShouldHoldTwinThreat(bot, sacrolash, threatRatio, IsAnySacrolashTank))
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
        if (ShouldHoldTwinThreat(bot, alythess, threatRatio, IsAlythessTank))
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
        Position const& position = PlayerbotAI::IsRanged(bot) ?
            EREDAR_TWINS_RANGED_CONFLAG_POSITION : EREDAR_TWINS_MELEE_CONFLAG_POSITION;

        if (bot->GetExactDist2d(position) <= 1.0f)
            return false;

        return MoveTo(
            SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else
    {
        constexpr float safeDistance = 10.0f;
        if (Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance))
        {
            float const distanceToPlayer = bot->GetExactDist2d(nearestPlayer);
            if (distanceToPlayer >= safeDistance)
                return false;

            botAI->InterruptSpell();
            return MoveAway(nearestPlayer, safeDistance - distanceToPlayer);
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
    if (bot->GetDistance2d(victim) >= safeDistance)
        return false;

    botAI->InterruptSpell();
    return MoveFromGroup(safeDistance);
}
