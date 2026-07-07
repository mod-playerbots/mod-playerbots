/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <array>
#include <cmath>

#include "SWPActions.h"
#include "SWPEncounter_Felmyst.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "Timer.h"

using namespace SunwellHelpers;

bool FelmystMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", felmyst))
    {
        return botAI->CastSpell("steady shot", felmyst);
    }

    return false;
}

bool FelmystMainTankPositionBossOnGroundAction::Execute(Event /*event*/)
{
    ClearFelmystDemonicVaporKiteState(bot);

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    if (AI_VALUE(Unit*, "current target") != felmyst)
        return Attack(felmyst);

    if (felmyst->GetVictim() == bot && bot->GetHealthPct() > 50.0f)
    {
        Position const& position = GetFelmystMainTankGroundPosition(bot);
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
                SUNWELL_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool FelmystPositionRangedOnGroundAction::Execute(Event /*event*/)
{
    ClearFelmystDemonicVaporKiteState(bot);

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    Position position;
    if (!TryGetFelmystRangedPosition(botAI, bot, felmyst, position))
        return false;

    return MoveInside(
        SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        FELMYST_RANGED_GROUP_RADIUS, MovementPriority::MOVEMENT_COMBAT);
}

bool FelmystPositionMeleeOnGroundAction::Execute(Event /*event*/)
{
    ClearFelmystDemonicVaporKiteState(bot);

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    Position position;
    if (!TryGetFelmystGroundStackPosition(
            botAI, bot, felmyst, FelmystGroundStack::Melee, position))
    {
        return false;
    }

    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 0.25f)
    {
        return MoveTo(
            SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
            position.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool FelmystRemoveEncapsulateAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_MAGE)
        return botAI->CanCastSpell("ice block", bot) && botAI->CastSpell("ice block", bot);
    else
        return botAI->CanCastSpell("divine shield", bot) && botAI->CastSpell("divine shield", bot);
}

bool FelmystRunAwayFromEncapsulatedPlayerAction::Execute(Event /*event*/)
{
    Player* encapsulateTarget = GetFelmystEncapsulateTarget(bot);
    if (!encapsulateTarget || encapsulateTarget == bot)
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    const FelmystGroundStack botStack = GetClosestFelmystGroundStack(botAI, bot, felmyst, bot);
    const FelmystGroundStack targetStack = GetClosestFelmystGroundStack(
        botAI, bot, felmyst, encapsulateTarget);

    if (botStack == FelmystGroundStack::None || targetStack == FelmystGroundStack::None ||
        botStack != targetStack)
    {
        return false;
    }

    auto const tryMoveToStack = [&](FelmystGroundStack stack)
    {
        Position position;
        if (!TryGetFelmystGroundStackPosition(botAI, bot, felmyst, stack, position))
            return false;

        return MoveInside(
            SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
            position.GetPositionZ(), FELMYST_RANGED_GROUP_RADIUS,
            MovementPriority::MOVEMENT_FORCED);
    };

    if (targetStack == FelmystGroundStack::Left || targetStack == FelmystGroundStack::Right)
    {
        if (tryMoveToStack(FelmystGroundStack::Melee))
            return true;

        return tryMoveToStack(targetStack == FelmystGroundStack::Left ?
            FelmystGroundStack::Right : FelmystGroundStack::Left);
    }

    Position leftPosition;
    Position rightPosition;
    if (!TryGetFelmystGroundStackPosition(
            botAI, bot, felmyst, FelmystGroundStack::Left, leftPosition) ||
        !TryGetFelmystGroundStackPosition(
            botAI, bot, felmyst, FelmystGroundStack::Right, rightPosition))
    {
        return false;
    }

    if (bot->GetExactDist2d(leftPosition.GetPositionX(), leftPosition.GetPositionY()) <=
        bot->GetExactDist2d(rightPosition.GetPositionX(), rightPosition.GetPositionY()))
    {
        if (tryMoveToStack(FelmystGroundStack::Left))
            return true;

        return tryMoveToStack(FelmystGroundStack::Right);
    }

    if (tryMoveToStack(FelmystGroundStack::Right))
        return true;

    return tryMoveToStack(FelmystGroundStack::Left);
}

bool FelmystMassDispelGasNovaAction::Execute(Event /*event*/)
{
    if (Player* gasNovaTarget = GetFelmystGasNovaDispelTarget(bot);
        gasNovaTarget && botAI->CanCastSpell("mass dispel", gasNovaTarget))
    {
        return botAI->CastSpell("mass dispel", gasNovaTarget);
    }

    return false;
}

bool FelmystAvoidDemonicVaporAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 20.0f;
    Unit* nearestTrail = bot->FindNearestCreature(
        static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR_TRAIL), searchRadius, true);
    Unit* nearestVapor = bot->FindNearestCreature(
        static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR), searchRadius, true);

    Unit* hazard = nearestTrail ? nearestTrail : nearestVapor;
    if (hazard)
    {
        constexpr float safeDistFromVapor = 15.0f;
        const float currentDistance = bot->GetDistance2d(hazard);
        if (currentDistance < safeDistFromVapor)
        {
            botAI->InterruptSpell();
            return MoveAway(hazard, safeDistFromVapor - currentDistance);
        }
    }

    return false;
}

bool FelmystKiteDemonicVaporAction::Execute(Event /*event*/)
{
    Position destination;
    if (!TryGetFelmystDemonicVaporKiteDestination(bot, destination))
        return false;

    const float distToDestination = bot->GetExactDist2d(
        destination.GetPositionX(), destination.GetPositionY());

    const float dX = destination.GetPositionX() - bot->GetPositionX();
    const float dY = destination.GetPositionY() - bot->GetPositionY();
    const float moveDist = std::min(3.5f, distToDestination);
    const float moveX = bot->GetPositionX() + (dX / distToDestination) * moveDist;
    const float moveY = bot->GetPositionY() + (dY / distToDestination) * moveDist;

    return MoveTo(
        SUNWELL_MAP_ID, moveX, moveY, destination.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool FelmystMoveToSafeFogLaneAction::Execute(Event /*event*/)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
    {
        _fogCrateStuckSampleMs = 0;
        return false;
    }

    FelmystFogOfCorruptionState fogState;
    const bool hasActiveFog =
        TryGetActiveFelmystFogOfCorruptionState(bot, felmyst, fogState);
    FelmystFogLane thirdPassLane = FelmystFogLane::None;
    const bool shouldRepositionAfterThirdPass = !hasActiveFog &&
        TryGetFelmystPostThirdPassWindow(felmyst, thirdPassLane);

    if (!hasActiveFog && !shouldRepositionAfterThirdPass)
    {
        _fogCrateStuckSampleMs = 0;
        return false;
    }

    std::array<Position, 3> destinations;
    uint8 destinationCount = 0;
    if (!TryGetFelmystFogSafeDestinations(
            bot, shouldRepositionAfterThirdPass ? thirdPassLane : fogState.lane,
            destinations, destinationCount))
    {
        _fogCrateStuckSampleMs = 0;
        return false;
    }

    LastMovement const& lastMove = AI_VALUE(LastMovement&, "last movement");
    bool trackedDestinationFound = false;
    for (uint8 index = 0; index < destinationCount; ++index)
    {
        Position const& destination = destinations[index];
        if (lastMove.priority != MovementPriority::MOVEMENT_FORCED ||
            lastMove.lastMoveToMapId != SUNWELL_MAP_ID ||
            Position(
                lastMove.lastMoveToX, lastMove.lastMoveToY,
                lastMove.lastMoveToZ).GetExactDist(destination) >
            FELMYST_FOG_DESTINATION_MATCH_DISTANCE)
        {
            continue;
        }

        trackedDestinationFound = true;
        if (TryTeleportStuckBotOntoCrate(destination))
            return true;

        break;
    }

    if (!trackedDestinationFound)
        _fogCrateStuckSampleMs = 0;

    if (shouldRepositionAfterThirdPass)
    {
        uint8 bestIndex = 0;
        float bestDistance = std::numeric_limits<float>::max();
        for (uint8 index = 0; index < destinationCount; ++index)
        {
            Position const& destination = destinations[index];
            const float distanceToFelmyst = felmyst->GetExactDist2d(
                destination.GetPositionX(), destination.GetPositionY());

            if (distanceToFelmyst < bestDistance)
            {
                bestDistance = distanceToFelmyst;
                bestIndex = index;
            }
        }

        Position const& destination = destinations[bestIndex];
        return MoveTo(
            SUNWELL_MAP_ID, destination.GetPositionX(), destination.GetPositionY(),
            destination.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    for (uint8 index = 0; index < destinationCount; ++index)
    {
        Position const& destination = destinations[index];
        if (MoveTo(
                SUNWELL_MAP_ID, destination.GetPositionX(), destination.GetPositionY(),
                destination.GetPositionZ(), false, false, false, false,
                MovementPriority::MOVEMENT_FORCED, true, false))
        {
            return true;
        }
    }

    return false;
}

bool FelmystMoveToSafeFogLaneAction::TryTeleportStuckBotOntoCrate(
    Position const& destination)
{
    constexpr float crateCollisionCheckDistance = 2.0f;
    constexpr float progressResetDistance = 1.0f;
    constexpr uint32 stuckTimeoutMs = 1500;

    const Position FELMYST_STUCK_CRATE_POSITION = { 1484.443f, 591.337f, 23.391f };
    const Position FELMYST_ON_CRATE_POSITION = { 1482.181f, 591.253f, 24.545f };

    if (bot->GetExactDist2d(
            FELMYST_STUCK_CRATE_POSITION.GetPositionX(),
            FELMYST_STUCK_CRATE_POSITION.GetPositionY()) >
        crateCollisionCheckDistance)
    {
        _fogCrateStuckSampleMs = 0;
        return false;
    }

    const uint32 now = getMSTime();
    const float distanceToDestination = bot->GetExactDist(
        destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ());

    if (!_fogCrateStuckSampleMs || _fogCrateStuckDestination.GetExactDist(destination) >
        FELMYST_FOG_DESTINATION_MATCH_DISTANCE)
    {
        _fogCrateStuckDestination = destination;
        _fogCrateStuckNearestDistance = distanceToDestination;
        _fogCrateStuckSampleMs = now;
        return false;
    }

    if (distanceToDestination + progressResetDistance < _fogCrateStuckNearestDistance)
    {
        _fogCrateStuckNearestDistance = distanceToDestination;
        _fogCrateStuckSampleMs = now;
        return false;
    }

    if (getMSTimeDiff(_fogCrateStuckSampleMs, now) < stuckTimeoutMs)
        return false;

    _fogCrateStuckSampleMs = 0;
    botAI->InterruptSpell();
    return bot->TeleportTo(
        SUNWELL_MAP_ID, FELMYST_ON_CRATE_POSITION.GetPositionX(),
        FELMYST_ON_CRATE_POSITION.GetPositionY(),
        FELMYST_ON_CRATE_POSITION.GetPositionZ(), bot->GetOrientation());
}

bool FelmystMeleeClearTargetAction::Execute(Event /*event*/)
{
    botAI->InterruptSpell();
    bot->AttackStop();
    context->GetValue<Unit*>("current target")->Set(nullptr);
    bot->SetSelection(ObjectGuid());
    return true;
}

bool FelmystKillCharmedPlayerAction::Execute(Event /*event*/)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    Player* charmedPlayer = GetFelmystCharmedTarget(botAI, bot, felmyst);
    if (!charmedPlayer)
        return false;

    if (AI_VALUE(Unit*, "current target") != charmedPlayer)
        return Attack(charmedPlayer);

    return false;
}
