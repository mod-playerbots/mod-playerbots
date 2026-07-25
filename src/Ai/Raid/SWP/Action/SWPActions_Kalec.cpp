/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "SWPEncounter_Kalec.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "TargetValue.h"
#include <algorithm>

using namespace SwpHelpers;

bool KalecgosTankPositionBossAction::Execute(Event event)
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return false;

    if (AI_VALUE(Unit*, "current target") != kalecgos)
        return Attack(kalecgos);

    // If the fight just started, taunt (if needed) before moving
    if (kalecgos->GetVictim() != bot && kalecgos->GetHealthPct() > 90.0f)
        return botAI->DoSpecificAction("taunt spell", event, true);

    Position const& position = KALECGOS_TANK_POSITION;
    float const distToPosition = bot->GetExactDist2d(
        position.GetPositionX(), position.GetPositionY());

    if (distToPosition > 3.0f)
    {
        float maxMoveDist = kalecgos->GetVictim() == bot ? 2.25f : 3.5f;
        float const moveDist = std::min(maxMoveDist, distToPosition);
        bool backwards = kalecgos->GetVictim() == bot;

        float const dX = position.GetPositionX() - bot->GetPositionX();
        float const dY = position.GetPositionY() - bot->GetPositionY();
        float const moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        float const moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(
            SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
            false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
    }

    // Once the fight is in progress, move to the tank position before taunting
    // during tank swaps to avoid turning the boss
    if (kalecgos->GetVictim() != bot)
        return botAI->DoSpecificAction("taunt spell", event, true);

    return false;
}

bool KalecgosEnterSpectralRiftAction::Execute(Event /*event*/)
{
    if (Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
        kalecgos && botAI->IsTank(bot))
    {
        Player* surfaceTank = GetKalecgosCurrentTank(bot);
        if (!surfaceTank)
            return false;

        if (surfaceTank == bot)
        {
            surfaceTank = GetKalecgosReplacementTank(bot);
            if (!surfaceTank)
                return false;
        }

        Position const& position = KALECGOS_TANK_POSITION;
        if (surfaceTank->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 3.0f ||
            kalecgos->GetVictim() != surfaceTank)
        {
            return false;
        }
    }

    constexpr float searchRadius = 75.0f;
    GameObject* rift = bot->FindNearestGameObject(
        static_cast<uint32>(SwpObjects::GO_SPECTRAL_RIFT), searchRadius, true);
    if (!rift)
        return false;

    if (rift->IsAtInteractDistance(*bot, rift->GetInteractionDistance()))
    {
        rift->Use(bot);
        return true;
    }

    float const targetDist = rift->GetInteractionDistance() - 0.5f;
    float const angle = rift->GetAngle(bot);
    float const destX = rift->GetPositionX() + std::cos(angle) * targetDist;
    float const destY = rift->GetPositionY() + std::sin(angle) * targetDist;

    return MoveTo(
        SWP_MAP_ID, destX, destY, rift->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool KalecgosDisperseRangedAction::Execute(Event /*event*/)
{
    if (!_initialRangedPositionReached)
    {
        Position const& initialPos = KALECGOS_INITIAL_RANGED_POSITION;
        constexpr float initialRangedRadius = 10.0f;

        if (bot->GetExactDist2d(initialPos.GetPositionX(), initialPos.GetPositionY()) <=
            initialRangedRadius)
        {
            _initialRangedPositionReached = true;
            return false;
        }

        return MoveInside(
            SWP_MAP_ID, initialPos.GetPositionX(), initialPos.GetPositionY(),
            initialPos.GetPositionZ(), initialRangedRadius, MovementPriority::MOVEMENT_COMBAT);
    }

    if (Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos"))
    {
        constexpr float safeDistFromDragon = 20.0f;
        constexpr uint32 minInterval = 0;
        if (bot->GetExactDist2d(kalecgos) < safeDistFromDragon)
            return FleePosition(kalecgos->GetPosition(), safeDistFromDragon, minInterval);
    }

    constexpr float safeDistFromPlayer = 6.0f;
    if (Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
    {
        constexpr uint32 minInterval = 1000;
        return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer, minInterval);
    }

    return false;
}

bool KalecgosRemoveArcaneBuffetAction::Execute(Event /*event*/)
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

bool KalecgosSathrovarrTankStandWithKalecAction::Execute(Event /*event*/)
{
    Unit* sathrovarr = AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor");
    if (!sathrovarr)
        return false;

    constexpr float searchRadius = 20.0f;
    Unit* kalec = bot->FindNearestCreature(
        static_cast<uint32>(SwpNpcs::NPC_KALECGOS_HUMANOID), searchRadius);

    if (!kalec || sathrovarr->GetVictim() != kalec)
        return false;

    Position const position = kalec->GetPosition();
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) < 3.0f)
        return false;

    return MoveTo(
        SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KalecgosReturnToSpectralRealmGroundAction::Execute(Event /*event*/)
{
    return bot->TeleportTo(
        SWP_MAP_ID, bot->GetPositionX(), bot->GetPositionY(),
        KALECGOS_SPECTRAL_REALM_Z, bot->GetOrientation());
}
